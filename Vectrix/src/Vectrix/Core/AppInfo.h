#ifndef VECTRIXWORKSPACE_APP_INFO_H
#define VECTRIXWORKSPACE_APP_INFO_H

#include <cstdint>
#include "Log.h"
#include "version.h"

/**
 * @file AppInfo.h
 * @brief Definition of the different version functions
 * @ingroup utils
 */

using Version = std::uint32_t;

namespace Vectrix {
    /**
     * @brief This function create a version from the different version number
     * @param major A major version number
     * @param minor A minor version number
     * @param patch A patch version number
     */
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

    /**
     * @brief This function return the os number of the version
     * @param version A version instance
     */
    constexpr static std::uint32_t getOS(const Version version) {
        return version >> 24 & 0xFF;
    }

    /**
     * @brief This function return the major number of the version
     * @param version A version instance
     */
    constexpr static std::uint32_t getMajor(const Version version) {
        return (version & 0x7F000000) >> 24U;
    }

    /**
     * @brief This function return the minor number of the version
     * @param version A version instance
     */
    constexpr static std::uint32_t getMinor(const Version version) {
        return (version & 0x00FFF000) >> 12U;
    }

    /**
     * @brief This function return the patch number of the version
     * @param version A version instance
     */
    constexpr static std::uint32_t getPatch(const Version version) {
        return version & 0x00000FFF;
    }

    /**
     * @brief This function return in text the version
     * @param version A version instance
     */
    static std::string toString(const Version version) {
        char r[32];
        sprintf(r,"%s-%u.%u.%u",getOS(version) ? "Windows" : "Linux",getMajor(version),getMinor(version),getPatch(version));
        return {r};
    }

    /**
     * @brief This class hold the information about the application
     */
    class ApplicationInfo {
    public:
        ApplicationInfo(const std::string &appName,Version appVersion);
        ApplicationInfo(const std::string &appName,std::uint32_t major, std::uint32_t minor, std::uint32_t patch);

        /**
         * @brief This function return the application name
         */
        [[nodiscard]] const char* getAppName() const;
        /**
         * @brief This function return the application version
         */
        [[nodiscard]] Version getAppVersion() const;
        /**
         * @brief This function return the engine name
         */
        [[nodiscard]] static constexpr auto getEngineName() {
            return m_engineName;
        }
        /**
         * @brief This function return the engine version
         */
        [[nodiscard]] static Version getEngineVersion() {
            return makeVersion(VC_VERSION_MAJOR,VC_VERSION_MINOR,VC_VERSION_PATCH);
        }
    private:
        void init(const std::string &appName, Version appVersion);
        std::string m_appName;
        Version m_appVersion;
        static constexpr auto m_engineName = "Vectrix";
    };
}

#endif //VECTRIXWORKSPACE_APP_INFO_H