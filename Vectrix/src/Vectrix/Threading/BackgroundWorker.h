#ifndef VECTRIXWORKSPACE_BACKGROUNDWORKER_H
#define VECTRIXWORKSPACE_BACKGROUNDWORKER_H

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

#include "Thread.h"
#include "Vectrix/Core/Log.h"

/**
 * @file BackgroundWorker.h
 * @brief Definition of BackgroundWorker, a thread handling requests whose results the main thread collects
 * @ingroup threading
 */

namespace Vectrix {
	/// @cond INTERNAL
	namespace detail {
		// Wake the Application's loop if it is waiting for events (Application::wakeUp), does nothing without one
		void wakeMainLoop();
	}
	/// @endcond

	/**
	 * @brief A thread of its own handling requests one after the other, whose results are collected by poll
	 *
	 * The usual pattern for slow or blocking work (a network service, a file being parsed...) whose results go
	 * to the main thread, as ImGui and the scene are main thread only: push a request, and call poll every frame
	 * (e.g. in OnUpdate) to handle what is ready. The application's loop is woken when a result is ready, in
	 * case it waits for events.
	 *
	 * The handler runs on the worker's thread. It takes a Request and returns a Result; it may also take the
	 * worker's stop flag as a second parameter, to give up a long task when the worker is destroyed. An
	 * exception it throws is logged and the request dropped.
	 * @code
	 * struct PriceRequest { std::string market; };
	 * struct PriceResult { std::string market; double price; };
	 *
	 * Vectrix::BackgroundWorker<PriceRequest, PriceResult> m_prices{"Prices", [](PriceRequest&& request) {
	 *     return PriceResult{request.market, fetchPrice(request.market)}; // blocking HTTP call
	 * }};
	 *
	 * void OnUpdate(const Vectrix::DeltaTime&) override {
	 *     m_prices.poll([this](PriceResult&& result) { m_shown[result.market] = result.price; });
	 * }
	 * @endcode
	 * @tparam Request What is pushed to the worker
	 * @tparam Result What the handler returns for each request (not void)
	 * @note Destroying the worker raises the stop flag, waits for the request being handled and drops the
	 *       others: destroy it before what its handler uses
	 * @see JobSystem for short computations spread over several threads
	 * @ingroup threading
	 */
	template<typename Request, typename Result>
	class BackgroundWorker {
		static_assert(!std::is_void_v<Result>, "A BackgroundWorker returns a result for each request");
	public:
		/// The flag a handler can watch to stop early: true once the worker is being destroyed
		using StopFlag = std::atomic<bool>;

		/**
		 * @brief Start the worker's thread
		 * @param name The thread's name, for debuggers and the profiler
		 * @param handler `Result(Request&&)` or `Result(Request&&, const StopFlag&)`, run on the worker's thread
		 */
		template<typename Handler>
			requires std::invocable<Handler&, Request&&> || std::invocable<Handler&, Request&&, const StopFlag&>
		BackgroundWorker(std::string name, Handler&& handler) {
			if constexpr (std::invocable<Handler&, Request&&, const StopFlag&>)
				m_handler = std::forward<Handler>(handler);
			else
				m_handler = [h = std::forward<Handler>(handler)](Request&& request, const StopFlag&) mutable { return h(std::move(request)); };
			m_thread = std::thread([this, name = std::move(name)] { threadLoop(name); });
		}

		~BackgroundWorker() {
			m_stopFlag.store(true, std::memory_order_relaxed);
			{
				const std::scoped_lock lock(m_mutex);
				m_stopping = true;
				m_requests.clear();
			}
			m_wake.notify_one();
			m_thread.join();
		}

		BackgroundWorker(const BackgroundWorker&) = delete;
		BackgroundWorker& operator=(const BackgroundWorker&) = delete;

		/**
		 * @brief Queue a request, handled after the ones pushed before it
		 * @note Any thread
		 */
		void push(Request request) {
			{
				const std::scoped_lock lock(m_mutex);
				m_requests.push_back(std::move(request));
			}
			m_wake.notify_one();
		}

		/**
		 * @brief Give each ready result to a function, in the order of their requests
		 * @param onResult Called with each result, on the calling thread (the main thread, usually)
		 * @return How many results were given
		 * @note One thread at a time
		 */
		template<std::invocable<Result&&> F>
		size_t poll(F&& onResult) {
			{
				const std::scoped_lock lock(m_mutex);
				m_resultsTaken.swap(m_results);
			}
			const size_t count = m_resultsTaken.size();
			for (Result& result : m_resultsTaken)
				std::invoke(onResult, std::move(result));
			m_resultsTaken.clear();
			return count;
		}

		/// @brief Take the oldest ready result, if any
		std::optional<Result> tryPop() {
			const std::scoped_lock lock(m_mutex);
			if (m_results.empty())
				return std::nullopt;
			Result result = std::move(m_results.front());
			m_results.pop_front();
			return result;
		}

		/// @brief Whether every request pushed so far was handled (their results may still wait for poll)
		[[nodiscard]] bool isIdle() const {
			const std::scoped_lock lock(m_mutex);
			return m_requests.empty() && !m_busy;
		}

		/// @brief How many requests are waiting, the one being handled not counted
		[[nodiscard]] size_t pendingRequests() const {
			const std::scoped_lock lock(m_mutex);
			return m_requests.size();
		}
	private:
		void threadLoop(const std::string& name) {
			setCurrentThreadName(name);
			std::unique_lock lock(m_mutex);
			while (true) {
				m_wake.wait(lock, [this] { return m_stopping || !m_requests.empty(); });
				if (m_stopping)
					return;
				Request request = std::move(m_requests.front());
				m_requests.pop_front();
				m_busy = true;
				lock.unlock();

				std::optional<Result> result;
				try {
					result.emplace(m_handler(std::move(request), m_stopFlag));
				} catch (const std::exception& exception) {
					VC_CORE_ERROR_NO_EXIT("Background worker '{}': a request threw, dropped: {}", name, exception.what());
				} catch (...) {
					VC_CORE_ERROR_NO_EXIT("Background worker '{}': a request threw, dropped", name);
				}

				lock.lock();
				m_busy = false;
				if (result && !m_stopping) {
					m_results.push_back(std::move(*result));
					lock.unlock();
					detail::wakeMainLoop();
					lock.lock();
				}
			}
		}

		std::function<Result(Request&&, const StopFlag&)> m_handler;
		mutable std::mutex m_mutex;
		std::condition_variable m_wake;
		std::deque<Request> m_requests;
		std::deque<Result> m_results;
		std::deque<Result> m_resultsTaken; ///< Only touched by poll's thread
		bool m_busy = false;
		bool m_stopping = false;
		StopFlag m_stopFlag{false};
		std::thread m_thread; ///< Last: started once everything above exists
	};
}

#endif //VECTRIXWORKSPACE_BACKGROUNDWORKER_H
