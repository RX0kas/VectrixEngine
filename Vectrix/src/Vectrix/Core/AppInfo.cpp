#include "AppInfo.h"

namespace Vectrix {
    ApplicationInfo::ApplicationInfo(const std::string &appName, const Version appVersion) {
        init(appName, appVersion);
    }

    ApplicationInfo::ApplicationInfo(const std::string &appName, const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch) {
        init(appName, makeVersion(major, minor, patch));
    }

    const char* ApplicationInfo::getAppName() const {
        return m_appName.c_str();
    }

    Version ApplicationInfo::getAppVersion() const {
        return m_appVersion;
    }

    void ApplicationInfo::init(const std::string &appName, const Version appVersion) {
        m_appName = appName;
        m_appVersion = appVersion;
    }
}
