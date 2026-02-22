#ifndef VECTRIXWORKSPACE_VERSION_H
#define VECTRIXWORKSPACE_VERSION_H

#include <cstdint>
#include "Log.h"

using Version = std::uint32_t;

namespace Vectrix {
    static Version makeVersion(const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch) {
        if (major>8U) {
            VC_CORE_ERROR("Major is too high to be fully registered");
        }
        if (minor>12U) {
            VC_CORE_ERROR("Minor is too high to be fully registered");
        }
        if (patch>12U) {
            VC_CORE_ERROR("Patch is too high to be fully registered");
        }
        return static_cast<uint32_t>(VC_PLATFORM_ID)<< 31U | major << 24U | minor << 12U | patch;
    }

    constexpr static std::uint32_t getMajor(const Version version) {
        return (version & 0x7F000000) >> 24U;
    }
    constexpr static std::uint32_t getMinor(const Version version) {
        return (version & 0x00FFF000) >> 12U;
    }
    constexpr static std::uint32_t getPatch(const Version version) {
        return version & 0x00000FFF;
    }

    static std::string toString(const Version version) {
        char r[32];
        sprintf(r,"%s-%u.%u.%u",VC_PLATFORM_ID ? "Windows" : "Linux",getMajor(version),getMinor(version),getPatch(version));
        return std::string(r);
    }

    class ApplicationInfo {
    public:
        ApplicationInfo(const std::string &appName,Version appVersion);
        ApplicationInfo(const std::string &appName,std::uint32_t major, std::uint32_t minor, std::uint32_t patch);
        [[nodiscard]] const char* getAppName() const;
        [[nodiscard]] Version getAppVersion() const;
        [[nodiscard]] static constexpr auto getEngineName() {
            return m_engineName;
        }
        [[nodiscard]] static Version getEngineVersion() {
            return makeVersion(0,1,2);
        }
    private:
        void init(const std::string &appName, const Version appVersion);
        std::string m_appName;
        Version m_appVersion;
        static constexpr auto m_engineName = "Vectrix";
    };
}

#endif //VECTRIXWORKSPACE_VERSION_H