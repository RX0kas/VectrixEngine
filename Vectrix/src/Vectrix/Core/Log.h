#ifndef VECTRIXWORKSPACE_LOG_H
#define VECTRIXWORKSPACE_LOG_H

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

/**
 * @file Log.h
 * @brief Definition of the Log class and of the logging macros
 * @ingroup core
 *
 * Logging is only compiled in on a debug build. On a release build every macro below
 * expands to nothing, so the calls cost nothing and can be left in place.
 */

namespace Vectrix {
	/**
	 * @brief Owns the two loggers used across the engine
	 *
	 * One logger is for the engine itself and one is for the application built on top of
	 * it, which is what lets the output tell them apart. Rather than using this class
	 * directly, prefer the `VC_CORE_*` and `VC_*` macros.
	 * @see VC_CORE_INFO
	 * @see VC_INFO
	 * @ingroup core
	 */
	class Log {
	public:
		/**
		 * @brief Create the loggers and install their sinks
		 * @pre Has to be called once before any logging macro, the entry point does it
		 */
		static void init();
#ifdef VC_DEBUG
		/**
		 * @brief Return the logger the engine writes to
		 * @return The shared engine logger
		 * @note Only available on a debug build
		 */
		static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }

		/**
		 * @brief Return the logger the application writes to
		 * @return The shared client logger
		 * @note Only available on a debug build
		 */
		static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
#endif
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Log macro
#ifdef VC_DEBUG
/**
 * @brief Report an engine error coming from a library, then stop the program
 * @param ... The format string and its arguments, in the fmt style
 * @warning This breaks into the debugger and aborts
 * @ingroup core
 */
#define VC_CORE_CRITICAL(...)   ::Vectrix::Log::getCoreLogger()->critical(__VA_ARGS__);VC_DEBUGBREAK();abort() // An error from library

/**
 * @brief Report an engine error caused by the calling code, then stop the program
 * @param ... The format string and its arguments, in the fmt style
 * @warning This breaks into the debugger and aborts
 * @see VC_CORE_ERROR_NO_EXIT
 * @ingroup core
 */
#define VC_CORE_ERROR(...)   ::Vectrix::Log::getCoreLogger()->error(__VA_ARGS__);VC_DEBUGBREAK();abort() // An error caused by the user/dev

/**
 * @brief Report an engine error the application can recover from
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_CORE_ERROR_NO_EXIT(...)   ::Vectrix::Log::getCoreLogger()->error(__VA_ARGS__); // An error caused by the user/dev by the app can continue running

/**
 * @brief Report something suspicious happening inside the engine
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_CORE_WARN(...)    ::Vectrix::Log::getCoreLogger()->warn(__VA_ARGS__)

/**
 * @brief Report a normal engine event worth knowing about
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_CORE_INFO(...)    ::Vectrix::Log::getCoreLogger()->info(__VA_ARGS__)

/**
 * @brief Report a detailed engine event, for following what the engine does step by step
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_CORE_TRACE(...)   ::Vectrix::Log::getCoreLogger()->trace(__VA_ARGS__)

/**
 * @brief Report an application error coming from a library, then stop the program
 * @param ... The format string and its arguments, in the fmt style
 * @warning This breaks into the debugger and aborts
 * @ingroup core
 */
#define VC_CRITICAL(...)   ::Vectrix::Log::getClientLogger()->critical(__VA_ARGS__);VC_DEBUGBREAK();abort()

/**
 * @brief Report an application error it can recover from
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_ERROR_NO_EXIT(...)   ::Vectrix::Log::getClientLogger()->error(__VA_ARGS__)

/**
 * @brief Report an application error, then stop the program
 * @param ... The format string and its arguments, in the fmt style
 * @warning This breaks into the debugger and aborts
 * @see VC_ERROR_NO_EXIT
 * @ingroup core
 */
#define VC_ERROR(...)   ::Vectrix::Log::getClientLogger()->error(__VA_ARGS__);VC_DEBUGBREAK();abort()

/**
 * @brief Report something suspicious happening in the application
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_WARN(...)    ::Vectrix::Log::getClientLogger()->warn(__VA_ARGS__)

/**
 * @brief Report a normal application event worth knowing about
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_INFO(...)    ::Vectrix::Log::getClientLogger()->info(__VA_ARGS__)

/**
 * @brief Report a detailed application event
 * @param ... The format string and its arguments, in the fmt style
 * @ingroup core
 */
#define VC_TRACE(...)   ::Vectrix::Log::getClientLogger()->trace(__VA_ARGS__)
#else
/// @cond INTERNAL
#define VC_CORE_CRITICAL(...)
#define VC_CORE_ERROR(...)
#define VC_CORE_ERROR_NO_EXIT(...)
#define VC_CORE_WARN(...)
#define VC_CORE_INFO(...)
#define VC_CORE_TRACE(...)
#define VC_CRITICAL(...)
#define VC_ERROR(...)
#define VC_ERROR_NO_EXIT(...)
#define VC_WARN(...)
#define VC_INFO(...)
#define VC_TRACE(...)
/// @endcond
#endif
#endif
