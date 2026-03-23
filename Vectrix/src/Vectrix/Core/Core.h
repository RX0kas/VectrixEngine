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
#else
	#define VC_ASSERT(x, ...)
	#define VC_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#include "memory"

#define VC_DELETE_OWN(var) {const auto* v = var.release();delete v;}

/**
 * @namespace Vectrix
 * @brief Vectrix Engine namespace
 */
namespace Vectrix {

	template<typename T>
	using Own = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Own<T> createOwn(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> createRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#endif