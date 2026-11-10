#include "vcpch.h"
#include "Thread.h"

#include <atomic>

#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
	namespace {
		// Read from any thread, written once by main() before any other thread exists
		std::atomic<std::thread::id> s_mainThread{std::this_thread::get_id()};
	}

	bool isMainThread() {
		return std::this_thread::get_id() == s_mainThread.load(std::memory_order_relaxed);
	}

	std::thread::id getMainThreadId() {
		return s_mainThread.load(std::memory_order_relaxed);
	}

	void setCurrentThreadName(const std::string_view name) {
		detail::setPlatformThreadName(name);
		Profiler::get().setThreadName(name);
	}

	namespace detail {
		void setMainThread() {
			s_mainThread.store(std::this_thread::get_id(), std::memory_order_relaxed);
			// Only for the profiler: on Linux the main thread's name is the process's, as ps and top show it
			Profiler::get().setThreadName("Main thread");
		}
	}
}
