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

/**
 * @brief A version packed into a single number
 *
 * It holds the platform it was built for, then the major, minor and patch numbers, which
 * is what lets a version be written to a file and compared later.
 * @see Vectrix::makeVersion
 * @ingroup utils
 */
using Version = std::uint32_t;

namespace Vectrix {
    /**
     * @brief This function create a version from the different version number
     * @param major A major version number
     * @param minor A minor version number
     * @param patch A patch version number
     * @return The three numbers and the platform packed together
     * @warning Major is capped at 8, minor and patch at 12, going over is reported as an error
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
     * @return The platform the version was built for, 0 for Linux and 1 for Windows
     */
    constexpr static std::uint32_t getOS(const Version version) {
        return version >> 24 & 0xFF;
    }

    /**
     * @brief This function return the major number of the version
     * @param version A version instance
     * @return The major part of the version
     */
    constexpr static std::uint32_t getMajor(const Version version) {
        return (version & 0x7F000000) >> 24U;
    }

    /**
     * @brief This function return the minor number of the version
     * @param version A version instance
     * @return The minor part of the version
     */
    constexpr static std::uint32_t getMinor(const Version version) {
        return (version & 0x00FFF000) >> 12U;
    }

    /**
     * @brief This function return the patch number of the version
     * @param version A version instance
     * @return The patch part of the version
     */
    constexpr static std::uint32_t getPatch(const Version version) {
        return version & 0x00000FFF;
    }

    /**
     * @brief This function return in text the version
     * @param version A version instance
     * @return The version as `Platform-major.minor.patch`
     */
    static std::string toString(const Version version) {
        char r[32];
        sprintf(r,"%s-%u.%u.%u",getOS(version) ? "Windows" : "Linux",getMajor(version),getMinor(version),getPatch(version));
        return {r};
    }

    /**
     * @brief Tell if something written with one version can be read by another
     *
     * The major numbers have to match, and the reader may not be older than the writer.
     * @param version1 The version doing the reading
     * @param version2 The version the content was written with
     * @return true when version1 can handle what version2 produced
     * @ingroup utils
     */
    static bool isCompatible(Version version1,Version version2) {
        if (getMajor(version1)!=getMajor(version2)) return false;

        if (getMinor(version1) < getMinor(version2)) return false;

        return true;
    }

    /**
     * @brief This class hold the information about the application
     *
     * It carries the name and the version of the application, next to the ones of the
     * engine. Set it with VC_SET_APP_INFO rather than building one by hand.
     * @see VC_SET_APP_INFO
     * @see Application::getAppInfo
     * @ingroup utils
     */
    class ApplicationInfo {
    public:
        /**
         * @brief Build the information from an already packed version
         * @param appName The name of the application
         * @param appVersion The version of the application
         */
        ApplicationInfo(const std::string &appName,Version appVersion);

        /**
         * @brief Build the information from the three version numbers
         * @param appName The name of the application
         * @param major The major part of its version
         * @param minor The minor part of its version
         * @param patch The patch part of its version
         */
        ApplicationInfo(const std::string &appName,std::uint32_t major, std::uint32_t minor, std::uint32_t patch);

        /**
         * @brief This function return the application name
         * @return The name of the application
         */
        [[nodiscard]] const char* getAppName() const;
        /**
         * @brief This function return the application version
         * @return The version of the application
         */
        [[nodiscard]] Version getAppVersion() const;
        /**
         * @brief This function return the engine name
         * @return The name of the engine, always `Vectrix`
         */
        [[nodiscard]] static constexpr auto getEngineName() {
            return m_engineName;
        }
        /**
         * @brief This function return the engine version
         * @return The version of the engine the application was built against
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