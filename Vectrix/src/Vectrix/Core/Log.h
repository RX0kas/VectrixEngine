#ifndef VECTRIXWORKSPACE_LOG_H
#define VECTRIXWORKSPACE_LOG_H

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Vectrix {
	class Log {
	public:
		static void init();
#ifdef VC_DEBUG
		static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
#endif
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Log macro
#ifdef VC_DEBUG
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
#else
#define VC_CORE_CRITICAL(...)
#define VC_CORE_ERROR(...)
#define VC_CORE_WARN(...)
#define VC_CORE_INFO(...)
#define VC_CORE_TRACE(...)
#define VC_CRITICAL(...)
#define VC_ERROR(...)
#define VC_WARN(...)
#define VC_INFO(...)
#define VC_TRACE(...)
#endif
#endif