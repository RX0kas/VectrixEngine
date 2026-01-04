#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Vectrix {
	class Vectrix_API Log
	{
	public:
		static void init();

		static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Log macro
#define VC_CHECK_VK_SUCCESS(x,...) \
    VkResult r = x; \
    if (r != VK_SUCCESS) { \
        ::Vectrix::Log::getCoreLogger()->error(__VA_ARGS__, r); \
        VC_DEBUGBREAK(); \
		abort(); \
    }

#define VC_CORE_CRITICAL(...)   ::Vectrix::Log::getCoreLogger()->critical(__VA_ARGS__);VC_DEBUGBREAK();abort()
#define VC_CORE_ERROR(...)   ::Vectrix::Log::getCoreLogger()->error(__VA_ARGS__);VC_DEBUGBREAK();abort()
#define VC_CORE_WARN(...)    ::Vectrix::Log::getCoreLogger()->warn(__VA_ARGS__)
#define VC_CORE_INFO(...)    ::Vectrix::Log::getCoreLogger()->info(__VA_ARGS__)
#define VC_CORE_TRACE(...)   ::Vectrix::Log::getCoreLogger()->trace(__VA_ARGS__)

#define VC_CRITICAL(...)   ::Vectrix::Log::getClientLogger()->critical(__VA_ARGS__)
#define VC_ERROR(...)   ::Vectrix::Log::getClientLogger()->error(__VA_ARGS__)
#define VC_WARN(...)    ::Vectrix::Log::getClientLogger()->warn(__VA_ARGS__)
#define VC_INFO(...)    ::Vectrix::Log::getClientLogger()->info(__VA_ARGS__)
#define VC_TRACE(...)   ::Vectrix::Log::getClientLogger()->trace(__VA_ARGS__)