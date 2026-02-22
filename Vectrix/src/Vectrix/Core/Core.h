#pragma once

#include <memory>

#ifdef VC_PLATFORM_WINDOWS
	#if VC_DYNAMIC_LINK
		#ifdef VC_BUILD_DLL
			#define Vectrix_API __declspec(dllexport)
		#else
			#define Vectrix_API __declspec(dllimport)
		#endif
	#endif
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

#ifndef Vectrix_API
	#define Vectrix_API
#endif

#ifdef VC_DEBUG
	#define VC_ASSERT(x, ...) { if(!(x)) { VC_ERROR("Assertion Failed: {0}", __VA_ARGS__); VC_DEBUGBREAK(); } }
	#define VC_CORE_ASSERT(x, ...) { if(!(x)) { VC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); VC_DEBUGBREAK(); } }
#else
	#define VC_ASSERT(x, ...)
	#define VC_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

/**
 * @namespace Vectrix
 * @brief Vectrix Engine namespace
 */
namespace Vectrix {

	template<typename T>
	using Own = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

}