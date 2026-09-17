#include "ProjectSerializer.h"

#include <fstream>

#include "Vectrix/Core/Log.h"
#include "Vectrix/Scene/Scene.h"
#include "Vectrix/Scene/SceneSerializer.h"
#include "Vectrix/Utils/Folders.h"
#include "Vectrix/Utils/Json.h"
#include "Vectrix/Settings/SettingsManager.h"

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
		VC_CORE_INFO("Creating project \"{}\" in {}", name.c_str(), directory.string().c_str());

		std::error_code ec;
		if (std::filesystem::exists(directory, ec) && !std::filesystem::is_empty(directory, ec)) {
			VC_CORE_WARN("Can't create project \"{}\": a project already exists at {}", name.c_str(), directory.string().c_str());
			return {ALREADY_EXISTS, "A project already exists at " + directory.string()};
		}

		if (!std::filesystem::create_directories(directory, ec) && ec) {
			VC_CORE_ERROR_NO_EXIT("Can't create project directory: {}", directory.string().c_str());
			return {UNKNOWN_ERROR, "Can't create the project directory: " + ec.message()};
		}

		createProjectFolders(directory);

		const std::filesystem::path startScene = std::filesystem::path("Scenes") / (name + ".vctx");
		Scene scene(name);
		if (SceneSerializer::saveScene((directory / startScene).string(), scene) != SUCCESS) {
			return {UNKNOWN_ERROR, "Can't write the starting scene of the project"};
		}

		std::ofstream file(directory / (name + ".vcproj"), std::ios::binary);
		if (!file) {
		VC_CORE_ERROR_NO_EXIT("Can't create project file in: {}", directory.string().c_str());
			return {UNKNOWN_ERROR, "Can't create the .vcProj file"};
		}

		const uint32_t engineVersion = ApplicationInfo::getEngineVersion();
		file.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
		file.write(reinterpret_cast<const char*>(&engineVersion), sizeof(engineVersion));
		file.write(reinterpret_cast<const char*>(&PROJECT_VERSION), sizeof(PROJECT_VERSION));
		writeString(file, name);
		writeString(file, startScene.generic_string());
		file.close();

		if (Json::save((directory / settingsFileName).string(), JsonObject{}) != SUCCESS) {
			return {UNKNOWN_ERROR, "Can't create the project settings file"};
		}

		VC_CORE_INFO("Project \"{}\" created successfully in {}", name.c_str(), directory.string().c_str());
		return {SUCCESS, {}};
	}

	ProjectLoadResult ProjectSerializer::loadProject(const std::filesystem::path& path) {
		VC_CORE_INFO("Loading project from {}", path.string().c_str());

		std::ifstream file(path, std::ios::binary);
		if (!file) {
			VC_CORE_ERROR_NO_EXIT("Can't find file: {}", path.string().c_str());
			return {.result = NOT_FOUND};
		}

		if (!validMagicNumber(file)) {
			VC_CORE_ERROR_NO_EXIT("The file {} is not a Vectrix project file", path.string().c_str());
			return {.result = WRONG_FILE};
		}

		if (!validVectrixVersion(file).has_value()) {
			VC_CORE_ERROR_NO_EXIT("The file {} is not made for this Vectrix version", path.string().c_str());
			return {.result = OUTDATED};
		}

		if (!validProjectVersion(file).has_value()) {
			VC_CORE_ERROR_NO_EXIT("The file {} is too old", path.string().c_str());
			return {.result = OUTDATED};
		}

		const std::optional<std::string> name = getString(file);
		if (!name.has_value()) {
			VC_CORE_ERROR_NO_EXIT("Can't load the name of the project from file: {}", path.string().c_str());
			return {.result = UNKNOWN_ERROR};
		}

		const std::optional<std::string> startScene = getString(file);
		if (!startScene.has_value()) {
			VC_CORE_ERROR_NO_EXIT("Can't load the starting scene of the project from file: {}", path.string().c_str());
			return {.result = UNKNOWN_ERROR};
		}

		VC_CORE_INFO("Project \"{}\" loaded successfully, starting scene: {}", name->c_str(), startScene->c_str());
		return {.result = SUCCESS, .name = name.value(), .startScenePath = startScene.value()};
	}
} // Vectrix
