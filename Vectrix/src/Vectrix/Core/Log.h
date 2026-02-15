#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Vectrix {
	class Vectrix_API Log
	{
	public:
		static void init();

		static Ref<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& getClientLogger() { return s_ClientLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}

// Log macro
#define VC_CORE_CRITICAL(...)   ::Vectrix::Log::getCoreLogger()->critical(__VA_ARGS__);VC_DEBUGBREAK();abort() // An error from library
#define VC_CORE_ERROR(...)   ::Vectrix::Log::getCoreLogger()->error(__VA_ARGS__);VC_DEBUGBREAK();abort() // An error caused by the user/dev
#define VC_CORE_WARN(...)    ::Vectrix::Log::getCoreLogger()->warn(__VA_ARGS__)
#define VC_CORE_INFO(...)    ::Vectrix::Log::getCoreLogger()->info(__VA_ARGS__)
#define VC_CORE_TRACE(...)   ::Vectrix::Log::getCoreLogger()->trace(__VA_ARGS__)

#define VC_CRITICAL(...)   ::Vectrix::Log::getClientLogger()->critical(__VA_ARGS__)
#define VC_ERROR(...)   ::Vectrix::Log::getClientLogger()->error(__VA_ARGS__)
#define VC_WARN(...)    ::Vectrix::Log::getClientLogger()->warn(__VA_ARGS__)
#define VC_INFO(...)    ::Vectrix::Log::getClientLogger()->info(__VA_ARGS__)
#define VC_TRACE(...)   ::Vectrix::Log::getClientLogger()->trace(__VA_ARGS__)