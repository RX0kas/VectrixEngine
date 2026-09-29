#ifndef VECTRIXWORKSPACE_PROFILER_H
#define VECTRIXWORKSPACE_PROFILER_H
#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <string>
#include <thread>

#include <unordered_set>
#include "Vectrix/Application.h"
#include "Vectrix/Utils/Json.h"

/**
 * @brief The version of the profiler data format
 *
 * Written into every profile file, so an old one can be told apart from a current one.
 * @ingroup debugtools
 */
#define VC_PROFILER_VERSION "1.1"

/**
* @file Profiler.h
* @brief Class that provides time analysis on how the application and the engine run
* @ingroup debugtools
*/

namespace Vectrix {
    /**
     * @brief A duration in microseconds, kept as a double so it does not lose precision
     * @ingroup debugtools
     */
    using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;
    /**
     * @brief Data obtained on the execution of a function
     */
    struct ProfilerResult
    {
        /// The name the scope was measured under
        const char* name;

        /// When the scope was entered, counted from the start of the session
        FloatingPointMicroseconds start;

        /// How long the scope took
        std::chrono::microseconds elapsedTime;

        /// Which thread the scope ran on
        uint32_t threadID;
    };

    /**
     * @brief Data on the current session
     */
    struct ProfilerSession
    {
        /// The name of the session, which ends up in the profile file
        const char* name;
    };

    /// @cond INTERNAL
    class Timer {
    public:
        Timer(const char* name);
        ~Timer();

        void stop(bool internal = false);
    private:
        const char* m_name;
        std::chrono::time_point<std::chrono::steady_clock> m_startTimepoint;
        bool m_stopped;
    };
    /// @endcond



    /**
     * @brief Main Profiler class
     */
    class Profiler
    {
    public:
        ~Profiler() {
            if (m_currentSession)
                endSession();
        }

        /// @cond INTERNAL
        void beginSession(const char* name, const char* filepath = "results.json") {
            if (m_currentSession)
                endSession(); // otherwise the previous session leaks and its file is never closed
            m_outputStream.open(filepath);
            writeHeader();
            m_currentSession = new ProfilerSession{ name };
        }

        void endSession() {
            std::vector<Timer*> timersToStop;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                timersToStop.assign(m_activeTimers.begin(), m_activeTimers.end());
                m_activeTimers.clear();
                delete m_currentSession;
                m_currentSession = nullptr;
            }
            for (Timer* timer : timersToStop)
                timer->stop(true);

            writeFooter();
            m_outputStream.close();
            m_profileCount = 0;
        }
        /// @endcond
        /**
         * @brief Return the profiler of the application
         * @return The single instance, created the first time it is asked for
         * @ingroup debugtools
         */
        static Profiler& get()
        {
            static Profiler instance;
            return instance;
        }
    private:
        Profiler() : m_currentSession(nullptr), m_profileCount(0) {}
        friend class Timer;

        std::mutex m_mutex{};
        std::unordered_set<Timer*> m_activeTimers{};

        void registerTimer(Timer* timer);
        void unregisterTimer(Timer* timer);

        void writeResultInternal(const ProfilerResult& result);
        void writeResult(const ProfilerResult& result);

        void writeHeader() {
            m_outputStream << R"({"data": {)";
            writeData();
            m_outputStream << R"(},"traceEvents":[)";
            m_outputStream.flush();
        }

        void writeFooter() {
            if (!m_outputStream.is_open()) return;
            m_outputStream << "]}";
            m_outputStream.flush();
        }

        void writeData();

        ProfilerSession* m_currentSession;
        std::ofstream m_outputStream;
        int m_profileCount;
    };

    /*!
        \def VC_PROFILER_SCOPE(name)
        @brief Can be used to more precicely describe what a function is doing
        @warning Recommended to be used inside a function marked with VC_PROFILER_FUNCTION()
    */
    /*!
        \def VC_PROFILER_FUNCTION()
        @brief Used to say that this function will be measured by the profiler
    */
}

#if VC_PROFILER_ENABLE
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
/**
 * @brief The name of the enclosing function, whatever the compiler calls it
 *
 * It is what VC_PROFILER_FUNCTION records a measurement under.
 * @see VC_PROFILER_FUNCTION
 * @ingroup debugtools
 */
#define VC_FUNC_NAME __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define VC_FUNC_NAME __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define VC_FUNC_NAME __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define VC_FUNC_NAME __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define VC_FUNC_NAME __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define VC_FUNC_NAME __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define VC_FUNC_NAME __func__
#else
/**
 * @brief The name of the enclosing function, whatever the compiler calls it
 *
 * It is what VC_PROFILER_FUNCTION records a measurement under.
 * @see VC_PROFILER_FUNCTION
 * @ingroup debugtools
 */
#define VC_FUNC_NAME "VC_FUNC_NAME unknown"
#endif
/**
 * @brief Start recording into a profile file
 * @param name The name of the session
 * @param filepath Where the recorded data is written
 * @note The entry point opens a session for startup, runtime and shutdown
 * @see VC_PROFILER_END_SESSION
 * @ingroup debugtools
 */
#define VC_PROFILER_BEGIN_SESSION(name, filepath) ::Vectrix::Profiler::get().beginSession(name, filepath)

/**
 * @brief Stop recording and close the profile file
 * @see VC_PROFILER_BEGIN_SESSION
 * @ingroup debugtools
 */
#define VC_PROFILER_END_SESSION() ::Vectrix::Profiler::get().endSession()

/**
 * @brief Measure how long the enclosing scope takes, under a name of your choosing
 * @param name The name the measurement is recorded under
 * @see VC_PROFILER_FUNCTION
 * @ingroup debugtools
 */
// Two levels so __LINE__ is expanded before pasting: timer##__LINE__ would name every timer "timer__LINE__"
#define VC_PROFILER_CONCAT_IMPL(a, b) a##b
#define VC_PROFILER_CONCAT(a, b) VC_PROFILER_CONCAT_IMPL(a, b)
#define VC_PROFILER_SCOPE(name) ::Vectrix::Timer VC_PROFILER_CONCAT(vcProfilerTimer, __LINE__)(name);

/**
 * @brief Measure how long the enclosing function takes, under its own name
 *
 * The usual way to instrument a function: put it on the first line and the whole call is
 * timed. It costs nothing on a release build.
 * @see VC_PROFILER_SCOPE
 * @ingroup debugtools
 */
#define VC_PROFILER_FUNCTION() VC_PROFILER_SCOPE(VC_FUNC_NAME)
#else
#define VC_PROFILER_BEGIN_SESSION(name, filepath)
#define VC_PROFILER_END_SESSION()
#define VC_PROFILER_SCOPE(name)
#define VC_PROFILER_FUNCTION()
#endif

#endif //VECTRIXWORKSPACE_PROFILER_H