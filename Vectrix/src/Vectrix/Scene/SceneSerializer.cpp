#include "SceneSerializer.h"

#include "Vectrix/Core/AppInfo.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    // TODO: Verify Hash
    SceneCreationData SceneSerializer::loadSceneFile(const std::string& path) {
        std::ifstream file(path,std::ios::binary);
        if (!file) {
            VC_CORE_ERROR_NO_EXIT("Can't find file: {}",path.c_str());
            return {};
        }

        if (!validMagicNumber(file)) {
            VC_CORE_ERROR_NO_EXIT("The file {} is not a Vectrix scene file",path.c_str());
            return {};
        }

        std::optional<uint32_t> vectrixVersion = validVectrixVersion(file);

        if (!vectrixVersion.has_value()) {
            VC_CORE_ERROR_NO_EXIT("The file {} is not made for this Vectrix version",path.c_str());
            return {};
        }

        std::optional<uint32_t> sceneVersion = validSceneVersion(file);
        if (!sceneVersion.has_value()) {
            VC_CORE_ERROR_NO_EXIT("The file {} is too old",path.c_str());
            return {};
        }


        SceneCreationData data{};
        data.file_version = sceneVersion.value();
        data.engine_version = vectrixVersion.value();

        std::optional<std::string> sceneName = getName(file);

        if (!sceneName.has_value()) {
            VC_CORE_ERROR_NO_EXIT("Can't load the name of the scene from file: {}",path.c_str());
            return {};
        }

        auto entities = readEntities(file);
        if (!entities.has_value()) {
            VC_CORE_ERROR_NO_EXIT("Can't load the entities of the scene from file: {}",path.c_str());
            return {};
        }
        data.entities = entities.value();

        data.loaded = true;
        return data;
    }

    bool SceneSerializer::validMagicNumber(std::ifstream& stream) {
        std::uint32_t fileMagicNumber;
        if (stream.read(reinterpret_cast<char*>(&fileMagicNumber), sizeof(fileMagicNumber)))
            return fileMagicNumber==MAGIC_NUMBER;

        return false;
    }

    std::optional<uint32_t> SceneSerializer::validVectrixVersion(std::ifstream &stream) {
        std::uint32_t fileVectrixVersion;
        if (stream.read(reinterpret_cast<char*>(&fileVectrixVersion), sizeof(fileVectrixVersion)) && isCompatible(ApplicationInfo::getEngineVersion(),fileVectrixVersion))
            return fileVectrixVersion;
        return std::nullopt;
    }

    std::optional<uint32_t> SceneSerializer::validSceneVersion(std::ifstream &stream) {
        std::uint32_t fileSceneVersion;
        if (stream.read(reinterpret_cast<char*>(&fileSceneVersion), sizeof(fileSceneVersion)) && isCompatible(SCENE_VERSION,fileSceneVersion))
            return fileSceneVersion;
        return std::nullopt;
    }

    std::optional<std::string> SceneSerializer::getName(std::ifstream& stream) {
        std::uint16_t len;
        if (!stream.read(reinterpret_cast<char*>(&len), sizeof(len)))
            return std::nullopt;

        std::string name(len, '\0');
        if (!stream.read(name.data(), len))
            return std::nullopt;

        return name;
    }

    std::optional<std::vector<EntityCreationData>> SceneSerializer::readEntities(std::ifstream &stream) {
        std::uint32_t entityCount;
        if (!stream.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount))) {
            return std::nullopt;
        }

        std::vector<EntityCreationData> datas(entityCount);
        for (std::uint32_t entitiesLoaded = 0; entitiesLoaded < entityCount; entitiesLoaded++) {
            std::optional<std::string> entityName = getName(stream);

            if (!entityName.has_value()) {
                return std::nullopt;
            }

            std::optional<std::vector<ComponentCreationData>> components = readComponents(stream);
            if (!components.has_value()) {
                return std::nullopt;
            }

            EntityCreationData data{};
            data.name = entityName.value();
            data.components = components.value();

            datas.push_back(data);
        }

        return datas;
    }

    std::optional<std::vector<ComponentCreationData>> SceneSerializer::readComponents(std::ifstream &stream) {
        std::uint16_t componentsCount;
        if (!stream.read(reinterpret_cast<char*>(&componentsCount), sizeof(componentsCount))) {
            return std::nullopt;
        }


        std::vector<ComponentCreationData> datas;
        for (std::uint16_t componentLoaded = 0; componentLoaded < componentsCount; componentLoaded++) {
            std::uint32_t type_id;
            if (!stream.read(reinterpret_cast<char*>(&type_id), sizeof(type_id))) {
                return std::nullopt;
            }

            std::uint32_t data_size;
            if (!stream.read(reinterpret_cast<char*>(&data_size), sizeof(data_size))) {
                return std::nullopt;
            }

            std::vector<std::byte> data(data_size);
            if (!stream.read(reinterpret_cast<char*>(data.data()), data_size)) {
                return std::nullopt;
            }

            ComponentCreationData creationData;
            creationData.type_id = type_id;
            creationData.data = data;

            datas.push_back(creationData);
        }

        return datas;
    }
} // Vectrix