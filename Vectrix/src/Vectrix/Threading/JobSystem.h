#ifndef VECTRIXWORKSPACE_JOBSYSTEM_H
#define VECTRIXWORKSPACE_JOBSYSTEM_H

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/**
 * @file JobSystem.h
 * @brief Definition of the JobSystem, the engine's pool of worker threads
 * @ingroup threading
 */

namespace Vectrix {
	class JobSystem;

	/**
	 * @brief What a submitted job (or group of jobs) is tracked with
	 *
	 * Copyable. A default-built handle tracks nothing: waiting on it returns at once.
	 * @see JobSystem::submit
	 * @ingroup threading
	 */
	class JobHandle {
	public:
		JobHandle() = default;
		/// @brief Whether every job of the handle has finished (true for an empty handle)
		[[nodiscard]] bool isDone() const { return !m_state || m_state->remaining.load(std::memory_order_acquire) == 0; }
	private:
		friend class JobSystem;
		struct State {
			std::atomic<uint32_t> remaining{0};
			std::mutex exceptionMutex;
			std::exception_ptr exception; ///< The first job's exception, rethrown by wait
		};
		explicit JobHandle(std::shared_ptr<State> state) : m_state(std::move(state)) {}
		std::shared_ptr<State> m_state;
	};

	/**
	 * @brief A pool of worker threads running jobs: short tasks split off the main thread's work
	 *
	 * The Application creates it before the window, with `engine.threading.workerCount` workers (0, the
	 * default, for one per hardware thread minus the main one), and gets it with JobSystem::get().
	 *
	 * - submit runs a function on a worker and gives back a JobHandle;
	 * - wait blocks until a handle's jobs are done. The waiting thread runs queued jobs meanwhile, so a job may
	 *   wait for jobs it submitted without blocking the pool;
	 * - parallelFor splits a loop over the workers and the calling thread, and returns once it's all done. It is
	 *   what the physics solver runs its passes with.
	 *
	 * Jobs are for computation. A job must not touch what is main thread only (ImGui, the layers, the scene's
	 * registry, the assets managers, the renderer's frame): use Application::runOnMainThread for that. For a
	 * long blocking task (network, a file dialog), use a BackgroundWorker, which has its own thread, rather than
	 * holding a worker.
	 * @code
	 * Vectrix::JobSystem& jobs = Vectrix::JobSystem::get();
	 * jobs.parallelFor(bodies.size(), 64, [&](size_t begin, size_t end) {
	 *     for (size_t i = begin; i < end; ++i)
	 *         integrate(bodies[i], dt);
	 * });
	 * @endcode
	 * @ingroup threading
	 */
	class JobSystem {
	public:
		/**
		 * @brief Start the workers
		 * @param workerCount How many threads, 0 for defaultWorkerCount()
		 */
		explicit JobSystem(uint32_t workerCount = 0);
		/// Runs the jobs still queued, then stops the workers
		~JobSystem();
		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;

		/**
		 * @brief The application's job system
		 * @note Only between the Application's construction and its destruction
		 */
		static JobSystem& get();
		/// @brief Whether there is an application job system (false before the Application, and in tests)
		static bool exists() { return s_instance != nullptr; }

		/// @brief One worker per hardware thread, minus the main thread's, and at least one
		[[nodiscard]] static uint32_t defaultWorkerCount();

		/// @brief How many worker threads run the jobs
		[[nodiscard]] uint32_t getWorkerCount() const { return static_cast<uint32_t>(m_workers.size()); }

		/**
		 * @brief Run a function on a worker
		 * @param job The function. An exception it throws is kept, and rethrown by wait
		 * @return The handle to wait on
		 */
		JobHandle submit(std::function<void()> job);

		/**
		 * @brief Block until the handle's jobs are done, running queued jobs meanwhile
		 * @param handle What to wait for
		 * @throw The exception of a job, if one threw
		 */
		void wait(const JobHandle& handle);

		/**
		 * @brief Block until no job is queued or running, running queued jobs meanwhile
		 *
		 * The Application calls it before tearing down what jobs may use (the graphics context, the window).
		 * @warning Not from a job: it would wait for itself
		 */
		void waitIdle();

		/**
		 * @brief Run fn over [0, count) in ranges of `grain`, spread over the workers and the calling thread
		 *
		 * Returns once every range is done. The ranges run in no particular order, possibly at the same time:
		 * fn must only write what its own range owns.
		 * @param count The number of items
		 * @param grain How many items a range holds at most (more means fewer, longer ranges; 0 is taken as 1)
		 * @param fn Called as fn(begin, end) for each range
		 * @throw The exception of a range, if one threw (the other ranges still run)
		 */
		template<typename Fn>
		void parallelFor(size_t count, size_t grain, Fn&& fn) {
			if (count == 0)
				return;
			grain = std::max<size_t>(grain, 1);
			const size_t rangeCount = (count + grain - 1) / grain;
			if (rangeCount == 1) {
				fn(size_t{0}, count);
				return;
			}

			// The helpers and this thread take the ranges from the same counter: no job per range, and whoever is
			// free takes the next one
			std::atomic<size_t> nextRange{0};
			auto runRanges = [&] {
				for (size_t range = nextRange.fetch_add(1, std::memory_order_relaxed); range < rangeCount;
				     range = nextRange.fetch_add(1, std::memory_order_relaxed)) {
					const size_t begin = range * grain;
					fn(begin, std::min(begin + grain, count));
				}
			};

			const auto helperCount = static_cast<uint32_t>(std::min<size_t>(rangeCount - 1, m_workers.size()));
			const JobHandle helpers = submitGroup(helperCount, runRanges);
			std::exception_ptr exception;
			try {
				runRanges();
			} catch (...) {
				exception = std::current_exception();
			}
			wait(helpers); // also before rethrowing: the helpers use this frame's variables
			if (exception)
				std::rethrow_exception(exception);
		}
	private:
		struct Job {
			std::function<void()> function;
			std::shared_ptr<JobHandle::State> state;
		};

		// Queue `count` copies of the same job under one handle
		JobHandle submitGroup(uint32_t count, const std::function<void()>& job);
		// Take a queued job, if any. m_mutex must be held
		bool tryPopLocked(Job& job);
		// Run a job and report it finished
		void run(Job& job);
		void workerLoop(uint32_t index);

		std::vector<std::thread> m_workers;
		std::mutex m_mutex;
		std::condition_variable m_wakeWorkers;
		std::condition_variable m_jobFinished; ///< For the threads in wait with nothing left to run
		std::deque<Job> m_queue;
		bool m_stopping = false;
		uint32_t m_runningJobs = 0; ///< Popped and not finished yet, for waitIdle

		static JobSystem* s_instance;
	};
}

#endif //VECTRIXWORKSPACE_JOBSYSTEM_H
