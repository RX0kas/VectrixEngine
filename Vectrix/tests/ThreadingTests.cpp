#include "Test.h"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

#include "Vectrix/Core/FixedTimestep.h"
#include "Vectrix/Threading/BackgroundWorker.h"
#include "Vectrix/Threading/JobSystem.h"
#include "Vectrix/Threading/Thread.h"

using namespace Vectrix;

VC_TEST("JobSystem: submit runs every job") {
	JobSystem jobs(4);
	std::atomic<int> sum{0};
	std::vector<JobHandle> handles;
	for (int i = 1; i <= 100; ++i)
		handles.push_back(jobs.submit([&sum, i] { sum += i; }));
	for (const JobHandle& handle : handles)
		jobs.wait(handle);
	VC_CHECK(sum == 5050);
	for (const JobHandle& handle : handles)
		VC_CHECK(handle.isDone());
}

VC_TEST("JobSystem: jobs run off the main thread unless it waits") {
	JobSystem jobs(2);
	// Not waited on: only a worker can run it (wait() may run a queued job on the waiting thread)
	std::atomic<bool> onMain{true};
	const JobHandle handle = jobs.submit([&onMain] { onMain = isMainThread(); });
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
	while (!handle.isDone() && std::chrono::steady_clock::now() < deadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	VC_REQUIRE(handle.isDone());
	VC_CHECK(!onMain);
	VC_CHECK(isMainThread());
}

VC_TEST("JobSystem: parallelFor covers each index once") {
	JobSystem jobs(4);
	for (const size_t count : {size_t{0}, size_t{1}, size_t{7}, size_t{1000}, size_t{100003}}) {
		for (const size_t grain : {size_t{0}, size_t{1}, size_t{64}, size_t{5000}}) {
			std::vector<std::atomic<int>> hits(count);
			jobs.parallelFor(count, grain, [&hits](size_t begin, size_t end) {
				for (size_t i = begin; i < end; ++i)
					hits[i].fetch_add(1, std::memory_order_relaxed);
			});
			bool once = true;
			for (const std::atomic<int>& hit : hits)
				once = once && hit.load() == 1;
			VC_CHECK(once);
		}
	}
}

VC_TEST("JobSystem: a job can wait for jobs it submitted, even with one worker") {
	// With one worker running the outer job, the inner ones can only run if wait() runs them itself
	JobSystem jobs(1);
	std::atomic<int> inner{0};
	jobs.wait(jobs.submit([&] {
		std::vector<JobHandle> children;
		for (int i = 0; i < 8; ++i)
			children.push_back(jobs.submit([&inner] { ++inner; }));
		for (const JobHandle& child : children)
			jobs.wait(child);
	}));
	VC_CHECK(inner == 8);
}

VC_TEST("JobSystem: nested parallelFor") {
	JobSystem jobs(3);
	std::atomic<int> total{0};
	jobs.parallelFor(16, 1, [&](size_t, size_t) {
		jobs.parallelFor(100, 10, [&](size_t begin, size_t end) { total += static_cast<int>(end - begin); });
	});
	VC_CHECK(total == 1600);
}

VC_TEST("JobSystem: an exception reaches wait, the pool keeps working") {
	JobSystem jobs(2);
	bool caught = false;
	try {
		jobs.wait(jobs.submit([] { throw std::runtime_error("job failed"); }));
	} catch (const std::runtime_error&) {
		caught = true;
	}
	VC_CHECK(caught);

	caught = false;
	std::atomic<int> ranges{0};
	try {
		jobs.parallelFor(64, 1, [&ranges](size_t begin, size_t) {
			++ranges;
			if (begin == 10)
				throw std::runtime_error("range failed");
		});
	} catch (const std::runtime_error&) {
		caught = true;
	}
	VC_CHECK(caught);

	std::atomic<bool> ran{false};
	jobs.wait(jobs.submit([&ran] { ran = true; }));
	VC_CHECK(ran);
}

VC_TEST("JobSystem: waitIdle returns once every job, nested ones included, is done") {
	JobSystem jobs(3);
	std::atomic<int> done{0};
	for (int i = 0; i < 20; ++i) {
		jobs.submit([&] {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			jobs.submit([&done] { ++done; }); // submitted while waitIdle may already be waiting
			++done;
		});
	}
	jobs.waitIdle();
	VC_CHECK(done == 40);
}

VC_TEST("JobSystem: destruction runs the queued jobs") {
	std::atomic<int> done{0};
	{
		JobSystem jobs(1);
		for (int i = 0; i < 50; ++i)
			jobs.submit([&done] { ++done; });
	}
	VC_CHECK(done == 50);
}

VC_TEST("BackgroundWorker: results come back in order, on the polling thread") {
	std::atomic<bool> handledOnMain{false};
	BackgroundWorker<int, int> worker("Test worker", [&handledOnMain](int&& value) {
		if (isMainThread())
			handledOnMain = true;
		return value * 2;
	});
	for (int i = 0; i < 100; ++i)
		worker.push(i);

	std::vector<int> results;
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
	while (results.size() < 100 && std::chrono::steady_clock::now() < deadline) {
		worker.poll([&results](int&& result) {
			VC_CHECK(isMainThread());
			results.push_back(result);
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	VC_CHECK(!handledOnMain);
	VC_REQUIRE(results.size() == 100);
	for (int i = 0; i < 100; ++i)
		VC_CHECK(results[static_cast<size_t>(i)] == i * 2);
	VC_CHECK(worker.isIdle());
	VC_CHECK(!worker.tryPop().has_value());
}

VC_TEST("BackgroundWorker: a throwing request is dropped, the next ones go on") {
	BackgroundWorker<int, int> worker("Test worker", [](int&& value) {
		if (value == 1)
			throw std::runtime_error("bad request");
		return value;
	});
	worker.push(1);
	worker.push(2);
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
	while (!worker.isIdle() && std::chrono::steady_clock::now() < deadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	const auto result = worker.tryPop();
	VC_REQUIRE(result.has_value());
	VC_CHECK(*result == 2);
	VC_CHECK(!worker.tryPop().has_value());
}

VC_TEST("BackgroundWorker: destruction stops a long request through the stop flag") {
	std::atomic<bool> started{false};
	std::atomic<bool> sawStop{false};
	const auto begin = std::chrono::steady_clock::now();
	{
		BackgroundWorker<int, int> worker("Test worker", [&](int&&, const std::atomic<bool>& stopping) {
			started = true;
			while (!stopping.load())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			sawStop = true;
			return 0;
		});
		worker.push(0);
		worker.push(1); // dropped
		while (!started)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	VC_CHECK(sawStop);
	VC_CHECK(std::chrono::steady_clock::now() - begin < std::chrono::seconds(5));
}

VC_TEST("FixedTimestep: steps keep pace with the time") {
	FixedTimestep clock(0.01, 100);
	uint32_t steps = 0;
	for (int frame = 0; frame < 1000; ++frame)
		steps += clock.advance(0.0137); // a frame time that doesn't divide the step
	// 13.7 s of frames: 1370 steps, the last one may still be in the accumulator (rounding)
	VC_CHECK(steps == 1369 || steps == 1370);
	VC_CHECK(clock.alpha() >= 0.0 && clock.alpha() < 1.0);
}

VC_TEST("FixedTimestep: a long frame is capped") {
	FixedTimestep clock(1.0 / 60.0, 8);
	VC_CHECK(clock.advance(5.0) == 8);
	VC_CHECK(clock.alpha() < 1.0); // the rest was dropped
	VC_CHECK(clock.advance(0.0) == 0);
	VC_CHECK(clock.advance(-1.0) == 0);
	clock.reset();
	VC_CHECK(clock.advance(1.0 / 60.0 + 1e-9) == 1);
}

VC_TEST("FixedTimestep: out of range settings keep the defaults") {
	FixedTimestep clock(0.0, 0);
	VC_CHECK(clock.step() == 1.0 / 60.0);
	VC_CHECK(clock.maxSteps() == 8);
}
