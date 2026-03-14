#include "Profiler.h"

#include "Vectrix/Rendering/RendererAPI.h"

namespace Vectrix {
    void Profiler::registerTimer(Timer* timer) {
        std::lock_guard lock(m_mutex);
        if (!m_currentSession) return;
        m_activeTimers.insert(timer);
    }

    void Profiler::unregisterTimer(Timer* timer) {
        std::lock_guard lock(m_mutex);
        m_activeTimers.erase(timer);
    }

    void Profiler::writeResult(const ProfilerResult& result) {
        std::lock_guard lock(m_mutex);
        if (!m_currentSession) return;
        if (!m_outputStream.is_open()) return;

        writeResultInternal(result);
    }

    void Profiler::writeResultInternal(const ProfilerResult& result) {
        if (!m_outputStream.is_open()) return;
        if (!m_currentSession) return;
        if (m_profileCount++ > 0)
            m_outputStream << ",";
        std::string name = result.name;
        std::replace(name.begin(), name.end(), '"', '\'');

        m_outputStream << '{';
        m_outputStream << R"("duration":)" << (result.elapsedTime.count()) << ',';
        m_outputStream << R"("name":")" << name << "\",";
        m_outputStream << R"("threadID":)" << result.threadID << ',';
        m_outputStream << R"("start":)" << result.start.count();
        m_outputStream << '}';

        m_outputStream.flush();
    }

    void Profiler::writeData() {
        const ApplicationInfo appInfo = Application::getAppInfo();
        m_outputStream << R"("profiler_version": ")" << VC_PROFILER_VERSION << "\",";
        m_outputStream << R"("engine_name": ")" << ApplicationInfo::getEngineName() << "\",";
        m_outputStream << R"("engine_build": ")" << toString(ApplicationInfo::getEngineVersion()) << "\",";
        m_outputStream << R"("app_name": ")" << appInfo.getAppName() << "\",";
        m_outputStream << R"("app_build": ")" << toString(appInfo.getAppVersion()) << "\",";
        const char* rendererAPI = RendererAPI::getAPI()==RendererAPI::API::Vulkan ? "Vulkan" : "None";
        m_outputStream << R"("graphic_api": ")" << rendererAPI << '"';
    }

    Timer::Timer(const char* name) : m_name(name), m_stopped(false) {
        m_startTimepoint = std::chrono::steady_clock::now();
        Profiler::get().registerTimer(this);
        if (!Profiler::get().m_currentSession)
            m_stopped = true;
    }

    Timer::~Timer() {
        if (!m_stopped)
            stop();
        Profiler::get().unregisterTimer(this);
    }

    void Timer::stop(const bool internal) {
        if (m_stopped) return;

        auto endTimepoint = std::chrono::steady_clock::now();
        auto highResStart = FloatingPointMicroseconds{ m_startTimepoint.time_since_epoch() };
        auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch();

        const uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

        if (internal)
            Profiler::get().writeResultInternal({ m_name, highResStart, elapsedTime, threadID });
        else
            Profiler::get().writeResult({ m_name, highResStart, elapsedTime, threadID });

        m_stopped = true;
    }
}
