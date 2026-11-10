#ifndef VECTRIXWORKSPACE_THREAD_H
#define VECTRIXWORKSPACE_THREAD_H

#include <string_view>
#include <thread>

/**
 * @file Thread.h
 * @brief Which thread is the main one, and thread names for debuggers and the profiler
 * @ingroup threading
 */

namespace Vectrix {
	/**
	 * @brief Whether the calling thread is the main thread, the one running the Application's loop
	 *
	 * The window, ImGui, the layers, the scene, the assets managers and the renderer's frame are main thread only.
	 * @ingroup threading
	 */
	[[nodiscard]] bool isMainThread();

	/**
	 * @brief The id of the main thread
	 * @ingroup threading
	 */
	[[nodiscard]] std::thread::id getMainThreadId();

	/**
	 * @brief Name the calling thread, as debuggers (gdb, Visual Studio, RenderDoc...) and the profiler show it
	 * @param name The name, in UTF-8. Linux keeps only the first 15 bytes for the debuggers; the profiler keeps it whole
	 * @ingroup threading
	 */
	void setCurrentThreadName(std::string_view name);

	/// @cond INTERNAL
	namespace detail {
		// Called by main() (EntryPoint.h): the main thread is the one that runs it. Before, the thread that
		// initialized the engine's statics is taken, which is the same in practice
		void setMainThread();
		// The OS part of setCurrentThreadName, one per platform (Platform/*/...Thread.cpp)
		void setPlatformThreadName(std::string_view name);
	}
	/// @endcond
}

/**
 * @brief Check, in a Debug build, that the engine code is called on the main thread
 * @ingroup threading
 */
#define VC_CORE_ASSERT_MAIN_THREAD() VC_CORE_ASSERT(::Vectrix::isMainThread(), "{} must be called on the main thread", __func__)

#endif //VECTRIXWORKSPACE_THREAD_H
