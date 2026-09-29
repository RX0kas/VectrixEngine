#include "ProjectSerializer.h"

#include <fstream>

#include "Vectrix/Core/Log.h"
#include "Vectrix/Scene/Scene.h"
#include "Vectrix/Scene/SceneSerializer.h"
#include "Vectrix/Utils/Folders.h"
#include "Vectrix/Utils/Json.h"
#include "Vectrix/Settings/SettingsManager.h"
#include "Vectrix/Utils/Path.h"

namespace Vectrix {
	bool ProjectSerializer::validMagicNumber(std::ifstream& stream) {
		std::uint32_t fileMagicNumber;
		if (stream.read(reinterpret_cast<char*>(&fileMagicNumber), sizeof(fileMagicNumber)))
			return fileMagicNumber == MAGIC_NUMBER;

		return false;
	}

	std::optional<uint32_t> ProjectSerializer::validVectrixVersion(std::ifstream& stream) {
		std::uint32_t fileVectrixVersion;
		if (stream.read(reinterpret_cast<char*>(&fileVectrixVersion), sizeof(fileVectrixVersion)) && isCompatible(ApplicationInfo::getEngineVersion(), fileVectrixVersion))
			return fileVectrixVersion;
		return std::nullopt;
	}

	std::optional<uint32_t> ProjectSerializer::validProjectVersion(std::ifstream& stream) {
		std::uint32_t fileProjectVersion;
		if (stream.read(reinterpret_cast<char*>(&fileProjectVersion), sizeof(fileProjectVersion)) && isCompatible(PROJECT_VERSION, fileProjectVersion))
			return fileProjectVersion;
		return std::nullopt;
	}

	std::optional<std::string> ProjectSerializer::getString(std::ifstream& stream) {
		std::uint16_t len;
		if (!stream.read(reinterpret_cast<char*>(&len), sizeof(len)))
			return std::nullopt;

		std::string str(len, '\0');
		if (!stream.read(str.data(), len))
			return std::nullopt;

		return str;
	}

	void ProjectSerializer::writeString(std::ofstream& stream, const std::string& str) {
		uint16_t len = static_cast<uint16_t>(str.size());
		stream.write(reinterpret_cast<const char*>(&len), sizeof(len));
		stream.write(str.data(), len);
	}

	std::pair<VectrixResult, std::string> ProjectSerializer::createProject(const std::filesystem::path& directory, const std::string& name) {
		VC_CORE_INFO("Creating project \"{}\" in {}", name.c_str(), toUtf8(directory).c_str());

		// The name becomes the .vcproj and starting scene file names: it must not be empty or leave the project folder
		if (name.empty() || name == "." || name == ".." || name.find_first_of("/\\") != std::string::npos) {
			VC_CORE_ERROR_NO_EXIT("Invalid project name \"{}\"", name.c_str());
			return {WRONG_FILE, "The project name can't be empty or contain '/' or '\\'"};
		}

		std::error_code ec;
		if (std::filesystem::exists(directory, ec) && !std::filesystem::is_empty(directory, ec)) {
			VC_CORE_WARN("Can't create project \"{}\": a project already exists at {}", name.c_str(), toUtf8(directory).c_str());
			return {ALREADY_EXISTS, "A project already exists at " + toUtf8(directory)};
		}

		if (!std::filesystem::create_directories(directory, ec) && ec) {
			VC_CORE_ERROR_NO_EXIT("Can't create project directory: {}", toUtf8(directory).c_str());
			return {UNKNOWN_ERROR, "Can't create the project directory: " + ec.message()};
		}

		createProjectFolders(directory);

		const std::filesystem::path startScene = std::filesystem::path("Scenes") / fromUtf8(name + ".vctx");
		Scene scene(name);
		if (SceneSerializer::saveScene(toUtf8(directory / startScene), scene) != SUCCESS) {
			return {UNKNOWN_ERROR, "Can't write the starting scene of the project"};
		}

		std::ofstream file(directory / fromUtf8(name + ".vcproj"), std::ios::binary);
		if (!file) {
			VC_CORE_ERROR_NO_EXIT("Can't create project file in: {}", toUtf8(directory).c_str());
			return {UNKNOWN_ERROR, "Can't create the .vcProj file"};
		}

		const uint32_t engineVersion = ApplicationInfo::getEngineVersion();
		file.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
		file.write(reinterpret_cast<const char*>(&engineVersion), sizeof(engineVersion));
		file.write(reinterpret_cast<const char*>(&PROJECT_VERSION), sizeof(PROJECT_VERSION));
		writeString(file, name);
		writeString(file, toGenericUtf8(startScene));
		file.close();
		if (!file) {
			VC_CORE_ERROR_NO_EXIT("Failed while writing the project file in: {}", toUtf8(directory).c_str());
			return {UNKNOWN_ERROR, "Can't write the .vcProj file"};
		}

		if (Json::save(toUtf8(directory / settingsFileName), JsonObject{}) != SUCCESS) {
			return {UNKNOWN_ERROR, "Can't create the project settings file"};
		}

		VC_CORE_INFO("Project \"{}\" created successfully in {}", name.c_str(), toUtf8(directory).c_str());
		return {SUCCESS, {}};
	}

	ProjectLoadResult ProjectSerializer::loadProject(const std::filesystem::path& path) {
		VC_CORE_INFO("Loading project from {}", toUtf8(path).c_str());

		std::ifstream file(path, std::ios::binary);
		if (!file) {
			VC_CORE_ERROR_NO_EXIT("Can't find file: {}", toUtf8(path).c_str());
			return {.result = NOT_FOUND};
		}

		if (!validMagicNumber(file)) {
			VC_CORE_ERROR_NO_EXIT("The file {} is not a Vectrix project file", toUtf8(path).c_str());
			return {.result = WRONG_FILE};
		}

		if (!validVectrixVersion(file).has_value()) {
			VC_CORE_ERROR_NO_EXIT("The file {} is not made for this Vectrix version", toUtf8(path).c_str());
			return {.result = OUTDATED};
		}

		if (!validProjectVersion(file).has_value()) {
			VC_CORE_ERROR_NO_EXIT("The file {} is too old", toUtf8(path).c_str());
			return {.result = OUTDATED};
		}

		const std::optional<std::string> name = getString(file);
		if (!name.has_value()) {
			VC_CORE_ERROR_NO_EXIT("Can't load the name of the project from file: {}", toUtf8(path).c_str());
			return {.result = UNKNOWN_ERROR};
		}

		const std::optional<std::string> startScene = getString(file);
		if (!startScene.has_value()) {
			VC_CORE_ERROR_NO_EXIT("Can't load the starting scene of the project from file: {}", toUtf8(path).c_str());
			return {.result = UNKNOWN_ERROR};
		}

		VC_CORE_INFO("Project \"{}\" loaded successfully, starting scene: {}", name->c_str(), startScene->c_str());
		return {.result = SUCCESS, .name = name.value(), .startScenePath = fromUtf8(startScene.value())};
	}
} // Vectrix
