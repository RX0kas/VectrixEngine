#ifndef VECTRIXWORKSPACE_CORE_H
#define VECTRIXWORKSPACE_CORE_H

#ifdef VC_PLATFORM_WINDOWS
#define VC_DEBUGBREAK() __debugbreak()
	#define VC_PLATFORM_ID 1
#else
	#ifdef VC_PLATFORM_LINUX
		#include <csignal>
		#define VC_DEBUGBREAK() raise(SIGTRAP)
		#define VC_PLATFORM_ID 0
	#else
		#error Vectrix only support windows and Linux
	#endif
#endif

#ifdef VC_DEBUG
	#define VC_ASSERT(x, ...) { if(!(x)) { VC_ERROR("Assertion Failed: {0}", __VA_ARGS__); VC_DEBUGBREAK(); } }
	#define VC_CORE_ASSERT(x, ...) { if(!(x)) { VC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); VC_DEBUGBREAK(); } }
	#ifndef VC_PROFILER_ENABLE
		#define VC_PROFILER_ENABLE 1
	#endif
	#define VC_TIMEOUT_SYNC 5'000'000'000ULL
#else
	#define VC_ASSERT(x, ...)
	#define VC_CORE_ASSERT(x, ...)
	#define VC_TIMEOUT_SYNC UINT64_MAX
#endif

#define BIT(x) (1 << x)

#include "memory"

/**
 * @namespace Vectrix
 * @brief Vectrix Engine namespace
 */
namespace Vectrix {}

#endif