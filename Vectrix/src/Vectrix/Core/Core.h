#ifndef VECTRIXWORKSPACE_CORE_H
#define VECTRIXWORKSPACE_CORE_H

/**
 * @file Core.h
 * @brief Platform detection, assertion and bit helper macros shared by the whole engine
 * @ingroup core
 */

#ifdef VC_PLATFORM_WINDOWS
/**
 * @brief Stop the execution and hand over to the attached debugger
 * @ingroup core
 */
#define VC_DEBUGBREAK() __debugbreak()

/**
 * @brief The identifier of the platform the engine was built for
 *
 * It is 1 on Windows and 0 on Linux.
 * @ingroup core
 */
	#define VC_PLATFORM_ID 1
#else
	#ifdef VC_PLATFORM_LINUX
		#include <csignal>
/**
 * @brief Stop the execution and hand over to the attached debugger
 * @ingroup core
 */
		#define VC_DEBUGBREAK() raise(SIGTRAP)

/**
 * @brief The identifier of the platform the engine was built for
 *
 * It is 1 on Windows and 0 on Linux.
 * @ingroup core
 */
		#define VC_PLATFORM_ID 0
	#else
		#error Vectrix only support windows and Linux
	#endif
#endif

#ifdef VC_DEBUG
/**
 * @brief Check a condition in application code and report it when it does not hold
 * @param x The condition that is expected to be true
 * @param ... The message logged when the check fails
 * @note Compiled out on a release build, so the condition is not evaluated there
 * @ingroup core
 */
	#define VC_ASSERT(x, ...) { if(!(x)) { VC_ERROR("Assertion Failed: {0}", __VA_ARGS__); VC_DEBUGBREAK(); } }

/**
 * @brief Check a condition in engine code and report it when it does not hold
 * @param x The condition that is expected to be true
 * @param ... The message logged when the check fails
 * @note Compiled out on a release build, so the condition is not evaluated there
 * @ingroup core
 */
	#define VC_CORE_ASSERT(x, ...) { if(!(x)) { VC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); VC_DEBUGBREAK(); } }
	#ifndef VC_PROFILER_ENABLE
/**
 * @brief Turns the profiler instrumentation on, it defaults to on for a debug build
 * @see VC_PROFILER_FUNCTION
 * @ingroup debugtools
 */
		#define VC_PROFILER_ENABLE 1
	#endif

/**
 * @brief How long the engine waits on a GPU fence before giving up, in nanoseconds
 *
 * A debug build uses a finite timeout so a hang gets caught, a release build waits
 * forever.
 * @ingroup core
 */
	#define VC_TIMEOUT_SYNC 5'000'000'000ULL
#else
/// @cond INTERNAL
	#define VC_ASSERT(x, ...)
	#define VC_CORE_ASSERT(x, ...)
	#define VC_TIMEOUT_SYNC UINT64_MAX
/// @endcond
#endif

/**
 * @brief Build a value with only the bit at the given position set
 *
 * It is what the EventCategory flags are made of.
 * @param x The position of the bit, starting at 0
 * @ingroup core
 */
#define BIT(x) (1 << x)

#include "memory"

/**
 * @namespace Vectrix
 * @brief Vectrix Engine namespace
 */
namespace Vectrix {}

#endif
