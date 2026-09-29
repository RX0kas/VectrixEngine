#ifndef VECTRIXWORKSPACE_FOLDERS_H
#define VECTRIXWORKSPACE_FOLDERS_H

#include <filesystem>
#include <sago/platform_folders.h>

#include "Vectrix/Application.h"
#include "Vectrix/Utils/Path.h"

namespace Vectrix {
	inline std::filesystem::path getConfigHome() { return fromUtf8(sago::getConfigHome()); } // sago returns UTF-8

	inline std::filesystem::path getDefaultConfigFolder() {
		return getConfigHome() / "Vectrix";
	}

	inline std::filesystem::path getConfigFolder() {
		return getDefaultConfigFolder() / fromUtf8(Application::getAppInfo().getAppName());
	}

	/**
	 * @brief Creates the standard sub-folders of a freshly created project
	 * @param projectDir The project's root directory
	 */
	inline void createProjectFolders(const std::filesystem::path& projectDir) {
		std::filesystem::create_directories(projectDir / "Assets");
		std::filesystem::create_directories(projectDir / "Scenes");
	}

	inline std::filesystem::path getVectrixStateFolder() {
		return fromUtf8(sago::getStateDir()) / "Vectrix";
	}
}
#endif //VECTRIXWORKSPACE_FOLDERS_H
