#ifndef VECTRIXWORKSPACE_PROJECTSERIALIZER_H
#define VECTRIXWORKSPACE_PROJECTSERIALIZER_H
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string>

#include "sago/platform_folders.h"

#include "Vectrix/Core/AppInfo.h"
#include "Vectrix/Utils/Result.h"

/**
 * @file ProjectSerializer.h
 * @brief Reading and writing of the binary .vcProj project files
 * @ingroup core
 */

/**
 * @brief The major part of the project file format version, bumped by a breaking change
 * @ingroup core
 */
#define PROJECT_VERSION_MAJOR 0

/**
 * @brief The minor part of the project file format version, bumped by an addition
 * @ingroup core
 */
#define PROJECT_VERSION_MINOR 1

/**
 * @brief The patch part of the project file format version, bumped by a fix
 * @ingroup core
 */
#define PROJECT_VERSION_PATCH 0

namespace Vectrix {
	const static std::filesystem::path DefaultVectrixProjectPath = std::filesystem::path(sago::getDocumentsFolder()) / "VectrixProject" ;

	/*
	[HEADER]
		uint32_t  magic;
		uint32_t  engine_version;
		uint32_t  file_version;
		uint16_t  name_len;
		char[]    name;
		uint16_t  start_scene_len;
		char[]    start_scene;      // relative to the project directory
	 */

	/**
	 * @brief Everything a .vcProj file contains, once it has been read
	 *
	 * Check #result before using any other field: when the file could not be read the
	 * rest is meaningless.
	 * @see ProjectSerializer::loadProject
	 * @ingroup core
	 */
	struct ProjectLoadResult {
		/// Tells whether the file could be read, and what went wrong otherwise
		VectrixResult result;

		/// The name of the project
		std::string name;

		/// The scene opened when the project loads, relative to the project directory
		std::filesystem::path startScenePath;
	};

	/**
	 * @brief Writes a project's .vcProj file, and reads one back
	 *
	 * The format is binary: a header carrying the magic number, the versions, the project
	 * name and the path of its starting scene.
	 * @ingroup core
	 */
	class ProjectSerializer {
	public:
		/**
		 * @brief Create a new project on disk: folders, an empty starting scene, the
		 *        .vcProj manifest and a default project-tier settings file
		 * @param directory The project's root directory, created if missing
		 * @param name The name of the project
		 * @return Whether it worked, and a human-readable message when it did not
		 * @note Fails with #ALREADY_EXISTS when @p directory already exists and is not empty
		 */
		static std::pair<VectrixResult, std::string> createProject(const std::filesystem::path& directory, const std::string& name);

		/**
		 * @brief Read a .vcProj file and return what it contains
		 * @param path The path of the file to read
		 * @return The content of the file, its `result` field telling whether it worked
		 */
		static ProjectLoadResult loadProject(const std::filesystem::path& path);
	private:
		static constexpr std::uint32_t MAGIC_NUMBER = 0x5643504A; // VCPJ
		static constexpr Version PROJECT_VERSION = static_cast<uint32_t>(VC_PLATFORM_ID) << 31U | static_cast<uint32_t>(PROJECT_VERSION_MAJOR) << 24U | static_cast<uint32_t>(PROJECT_VERSION_MINOR) << 12U | static_cast<uint32_t>(PROJECT_VERSION_PATCH);

		static bool validMagicNumber(std::ifstream& stream);
		static std::optional<uint32_t> validVectrixVersion(std::ifstream& stream);
		static std::optional<uint32_t> validProjectVersion(std::ifstream& stream);
		static std::optional<std::string> getString(std::ifstream& stream);
		static void writeString(std::ofstream& stream, const std::string& str);
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_PROJECTSERIALIZER_H
