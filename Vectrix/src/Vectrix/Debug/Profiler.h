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
 * @see Vectrix::Profiler::isCompatible
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

        /**
         * @brief This function return true if the file given is compatible with the current version of the profiler
         * @param filePath path of a profiler data file
         * @return true if the file is compatible
         */
        static bool isCompatible(const std::string &filePath) {
            if (!isValidFormat(filePath)) return false;
            std::pair<VectrixResult, JsonValue> file = Json::load(filePath);

            if (file.first!=SUCCESS) {
                return false;
            }

            JsonValue root = file.second;
            std::string profiler_version = root["data"]["profiler_version"].getString();
            const size_t pointPos = profiler_version.find('.');
            const std::string majorStr = profiler_version.substr(0, pointPos);
            const std::string minorStr = profiler_version.substr(pointPos + 1);
            const std::string currentMajorStr = static_cast<std::string>(VC_PROFILER_VERSION).substr(0,pointPos);
            const std::string currentMinorStr = static_cast<std::string>(VC_PROFILER_VERSION).substr(pointPos+1,static_cast<std::string>(VC_PROFILER_VERSION).size()-pointPos);


            if (std::stoi(majorStr)!=std::stoi(currentMajorStr)) return false;

            if (std::stoi(currentMinorStr) < std::stoi(minorStr)) return false;

            return true;
        }

        /**
         * @brief Tell if a file is shaped like a profiler data file at all
         * @param filePath The path of the file to check
         * @return true when the file has the expected structure
         * @note Being valid does not mean being compatible, see isCompatible
         * @see isCompatible
         */
        static bool isValidFormat(const std::string &filePath) {
            std::pair<VectrixResult, JsonValue> file = Json::load(filePath);

            if (file.first!=SUCCESS) {
                return false;
            }

            JsonValue root = file.second;
            if (!root.contains("data")) {
                return false;
            }
            JsonValue data = root["data"];
            if (!data.isType<JsonObject>()) {
                return false;
            }

            if (!root.contains("traceEvents")) {
                return false;
            }

            JsonValue traceEvents = root["traceEvents"];
            if (!traceEvents.isType<JsonArray>()) {
                return false;
            }

            return true;
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
#define VC_PROFILER_SCOPE(name) ::Vectrix::Timer timer##__LINE__(name);

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