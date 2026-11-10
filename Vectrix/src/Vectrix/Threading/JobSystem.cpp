#include "vcpch.h"
#include "JobSystem.h"

#include <format>

#include "Thread.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
	JobSystem* JobSystem::s_instance = nullptr;

	JobSystem::JobSystem(uint32_t workerCount) {
		VC_PROFILER_FUNCTION();
		if (workerCount == 0)
			workerCount = defaultWorkerCount();
		// The first one is the application's (Application creates it before anything could use one)
		if (!s_instance)
			s_instance = this;

		m_workers.reserve(workerCount);
		for (uint32_t i = 0; i < workerCount; ++i)
			m_workers.emplace_back([this, i] { workerLoop(i); });
		VC_CORE_INFO("Job system started with {} worker thread(s)", workerCount);
	}

	JobSystem::~JobSystem() {
		VC_PROFILER_FUNCTION();
		{
			const std::scoped_lock lock(m_mutex);
			m_stopping = true;
		}
		m_wakeWorkers.notify_all();
		for (std::thread& worker : m_workers)
			worker.join();
		if (s_instance == this)
			s_instance = nullptr;
	}

	JobSystem& JobSystem::get() {
		VC_CORE_ASSERT(s_instance, "There is no job system: the Application creates it");
		return *s_instance;
	}

	uint32_t JobSystem::defaultWorkerCount() {
		const unsigned hardwareThreads = std::thread::hardware_concurrency(); // 0 when unknown
		return hardwareThreads > 1 ? hardwareThreads - 1 : 1;
	}

	JobHandle JobSystem::submit(std::function<void()> job) {
		auto state = std::make_shared<JobHandle::State>();
		state->remaining.store(1, std::memory_order_relaxed);
		{
			const std::scoped_lock lock(m_mutex);
			m_queue.push_back({std::move(job), state});
		}
		m_wakeWorkers.notify_one();
		return JobHandle(std::move(state));
	}

	JobHandle JobSystem::submitGroup(const uint32_t count, const std::function<void()>& job) {
		if (count == 0)
			return {};
		auto state = std::make_shared<JobHandle::State>();
		state->remaining.store(count, std::memory_order_relaxed);
		{
			const std::scoped_lock lock(m_mutex);
			for (uint32_t i = 0; i < count; ++i)
				m_queue.push_back({job, state});
		}
		if (count == 1)
			m_wakeWorkers.notify_one();
		else
			m_wakeWorkers.notify_all();
		return JobHandle(std::move(state));
	}

	void JobSystem::wait(const JobHandle& handle) {
		if (!handle.m_state)
			return;
		const std::shared_ptr<JobHandle::State>& state = handle.m_state;
		std::unique_lock lock(m_mutex);
		while (state->remaining.load(std::memory_order_acquire) != 0) {
			// Run what is queued instead of idling: the jobs waited on may be among them, and a job waiting for
			// the jobs it submitted can't block the pool that way
			Job job;
			if (tryPopLocked(job)) {
				lock.unlock();
				run(job);
				lock.lock();
				continue;
			}
			// Nothing queued: the jobs waited on are running on other threads, which notify when one ends
			m_jobFinished.wait(lock);
		}
		lock.unlock();

		const std::scoped_lock exceptionLock(state->exceptionMutex);
		if (state->exception)
			std::rethrow_exception(state->exception);
	}

	void JobSystem::waitIdle() {
		VC_PROFILER_FUNCTION();
		std::unique_lock lock(m_mutex);
		while (!m_queue.empty() || m_runningJobs != 0) {
			Job job;
			if (tryPopLocked(job)) {
				lock.unlock();
				run(job);
				lock.lock();
				continue;
			}
			m_jobFinished.wait(lock);
		}
	}

	bool JobSystem::tryPopLocked(Job& job) {
		if (m_queue.empty())
			return false;
		job = std::move(m_queue.front());
		m_queue.pop_front();
		++m_runningJobs;
		return true;
	}

	void JobSystem::run(Job& job) {
		try {
			job.function();
		} catch (...) {
			const std::scoped_lock lock(job.state->exceptionMutex);
			if (!job.state->exception)
				job.state->exception = std::current_exception();
		}
		job.function = nullptr; // what it captured goes before the waiters are released
		job.state->remaining.fetch_sub(1, std::memory_order_acq_rel);
		{
			// Under the lock, so a thread between its check and its wait in wait()/waitIdle() can't miss the notification
			const std::scoped_lock lock(m_mutex);
			--m_runningJobs;
		}
		m_jobFinished.notify_all();
	}

	void JobSystem::workerLoop(const uint32_t index) {
		setCurrentThreadName(std::format("Vectrix Worker {}", index + 1));
		std::unique_lock lock(m_mutex);
		while (true) {
			Job job;
			if (tryPopLocked(job)) {
				lock.unlock();
				{
					VC_PROFILER_SCOPE("Job");
					run(job);
				}
				lock.lock();
				continue;
			}
			// The queue is empty: stop once asked to, the jobs submitted before having all run
			if (m_stopping)
				return;
			m_wakeWorkers.wait(lock);
		}
	}
}
