#include "SceneSerializer.h"

#include "Entity.h"
#include "Vectrix/Scene/Component.h"
#include "Vectrix/Application.h"
#include "Vectrix/Core/AppInfo.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    namespace {
        /// Bytes left between the read position and the end of the file: sizes read from the file are
        /// checked against it before allocating, so a corrupt size can't ask for gigabytes
        std::uint64_t remainingBytes(std::ifstream& stream) {
            const std::streampos position = stream.tellg();
            stream.seekg(0, std::ios::end);
            const std::streampos end = stream.tellg();
            stream.seekg(position);
            return end > position ? static_cast<std::uint64_t>(end - position) : 0;
        }

        void writeString(std::ofstream& file, const std::string& str) {
            const auto len = static_cast<uint16_t>(str.size());
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(str.data(), len);
        }
    }

    // TODO: Verify Hash
    SceneCreationData SceneSerializer::loadSceneFile(const std::string& path) {
        std::ifstream file(path,std::ios::binary);
        if (!file) {
            VC_CORE_ERROR_NO_EXIT("Can't find file: {}",path.c_str());
            return {.result = NOT_FOUND};
        }

        if (!validMagicNumber(file)) {
            VC_CORE_ERROR_NO_EXIT("The file {} is not a Vectrix scene file",path.c_str());
            return {.result = WRONG_FILE};
        }

        std::optional<uint32_t> vectrixVersion = validVectrixVersion(file);

        if (!vectrixVersion.has_value()) {
            VC_CORE_ERROR_NO_EXIT("The file {} is not made for this Vectrix version",path.c_str());
            return {.result = OUTDATED};
        }

        std::optional<uint32_t> sceneVersion = validSceneVersion(file);
        if (!sceneVersion.has_value()) {
            VC_CORE_ERROR_NO_EXIT("The file {} is too old",path.c_str());
            return {.result = OUTDATED};
        }


        SceneCreationData data{};
        data.file_version = sceneVersion.value();
        data.engine_version = vectrixVersion.value();

        std::optional<std::string> sceneName = getString(file);

        if (!sceneName.has_value()) {
            VC_CORE_ERROR_NO_EXIT("Can't load the name of the scene from file: {}",path.c_str());
            return {.result = UNKNOWN_ERROR};
        }
        data.name = sceneName.value();

        auto entities = readEntities(file);
        if (!entities.has_value()) {
            VC_CORE_ERROR_NO_EXIT("Can't load the entities of the scene from file: {}",path.c_str());
            return {.result = UNKNOWN_ERROR};
        }
        data.entities = entities.value();

        data.result = SUCCESS;
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

    std::optional<std::string> SceneSerializer::getString(std::ifstream& stream) {
        std::uint16_t len;
        if (!stream.read(reinterpret_cast<char*>(&len), sizeof(len)))
            return std::nullopt;

        std::string str(len, '\0');
        if (!stream.read(str.data(), len))
            return std::nullopt;

        return str;
    }

    std::optional<std::vector<EntityCreationData>> SceneSerializer::readEntities(std::ifstream &stream) {
        std::uint32_t entityCount;
        if (!stream.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount))) {
            return std::nullopt;
        }

        // Every entity takes at least its name length and component count (4 bytes)
        if (entityCount > remainingBytes(stream) / 4) {
            return std::nullopt;
        }

        std::vector<EntityCreationData> datas(entityCount);
        for (std::uint32_t entitiesLoaded = 1; entitiesLoaded <= entityCount; entitiesLoaded++) {
            std::optional<std::string> entityName = getString(stream);

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

            datas[entitiesLoaded-1] = data;
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

            if (data_size > remainingBytes(stream)) {
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

    VectrixResult SceneSerializer::saveScene(const std::string &path, Scene& scene) {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            VC_CORE_ERROR_NO_EXIT("Can't save file: {}", path.c_str());
            return UNKNOWN_ERROR;
        }

        // Header
        uint32_t engineVersion = ApplicationInfo::getEngineVersion();
        file.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
        file.write(reinterpret_cast<const char*>(&engineVersion), sizeof(engineVersion));
        file.write(reinterpret_cast<const char*>(&SCENE_VERSION), sizeof(SCENE_VERSION));

        // Name
        writeString(file,scene.getName());

        auto view = scene.m_registry.view<InformationComponent>();
        uint32_t entityCount = view.size();
        file.write(reinterpret_cast<const char*>(&entityCount), sizeof(entityCount));
        for (const entt::entity& entity : view) {
            std::shared_ptr<Entity> e = scene.getEntity(entity);
            writeString(file,e->getComponent<InformationComponent>().name);

            uint16_t componentCount = 1;
            if (e->hasComponent<CameraComponent>()) componentCount++;
            if (e->hasComponent<MeshRendererComponent>()) componentCount++;
            file.write(reinterpret_cast<const char*>(&componentCount), sizeof(componentCount));

            // Components
            // TransformComponent
            TransformComponent transform = e->getComponent<TransformComponent>();
            uint32_t hashTransform = entt::type_hash<TransformComponent>::value();
            uint32_t transformSize = sizeof(TransformComponent);
            file.write(reinterpret_cast<const char*>(&hashTransform), sizeof(hashTransform));
            file.write(reinterpret_cast<const char*>(&transformSize), sizeof(transformSize));
            file.write(reinterpret_cast<const char*>(&transform), sizeof(transform));

            if (e->hasComponent<CameraComponent>()) {
                CameraComponent camera = e->getComponent<CameraComponent>();
                uint32_t hashCamera = entt::type_hash<CameraComponent>::value();
                uint32_t cameraComponentSize = sizeof(float)*4;
                file.write(reinterpret_cast<const char*>(&hashCamera), sizeof(hashCamera));
                file.write(reinterpret_cast<const char*>(&cameraComponentSize), sizeof(cameraComponentSize));

                // Data
                float fov = camera.camera.getFOV();
                file.write(reinterpret_cast<const char*>(&fov), sizeof(float));

                float camNear = camera.camera.getCamNear();
                file.write(reinterpret_cast<const char*>(&camNear), sizeof(float));

                float camFar = camera.camera.getCamFar();
                file.write(reinterpret_cast<const char*>(&camFar), sizeof(float));

                const float aspect = camera.camera.hasCustomAspect() ? camera.camera.getAspect() : -1.0f;
                file.write(reinterpret_cast<const char*>(&aspect), sizeof(float));
            }

            if (e->hasComponent<MeshRendererComponent>()) {
                auto& mesh = e->getComponent<MeshRendererComponent>();
                uint32_t hashMesh = entt::type_hash<MeshRendererComponent>::value();

                // The editor lets a component be saved before all its assets are set: an unset asset is an empty id
                const std::string meshId = mesh.mesh ? mesh.mesh->getID() : std::string();
                const std::string textureId = mesh.texture ? mesh.texture->getID() : std::string();
                const std::string shaderId = mesh.shader ? mesh.shader->getID() : std::string();

                uint32_t meshSize = sizeof(uint16_t) + meshId.size()
                                  + sizeof(uint16_t) + textureId.size()
                                  + sizeof(uint16_t) + shaderId.size() + sizeof(bool);
                file.write(reinterpret_cast<const char*>(&hashMesh), sizeof(hashMesh));
                file.write(reinterpret_cast<const char*>(&meshSize), sizeof(meshSize));
                writeString(file, meshId);
                writeString(file, textureId);
                writeString(file, shaderId);
                bool isEnable = mesh.isEnable();
                file.write(reinterpret_cast<const char*>(&isEnable),sizeof(bool));
            }
        }

        if (!file.good()) {
            VC_CORE_ERROR_NO_EXIT("Failed while writing: {}", path.c_str());
            return UNKNOWN_ERROR;
        }
        return SUCCESS;
    }
} // Vectrix