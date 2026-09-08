#include "Scene.h"

#include <utility>

#include "Component.h"
#include "Entity.h"
#include "Vectrix/Assets/AssetsManager.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    /// @cond INTERNAL
    std::string getString(const std::vector<std::byte>& bytes, uint32_t& offset) {
        auto startByte = bytes.begin() + offset;
        std::vector<std::byte> lenData = {startByte, startByte + sizeof(uint16_t)};
        auto* idLen = reinterpret_cast<uint16_t*>(lenData.data());
        auto startId = startByte + sizeof(uint16_t);
        std::vector<std::byte> idData = {startId, startId + *idLen};
        char* idPtr = reinterpret_cast<char*>(idData.data());
        std::string id = {idPtr, idPtr + *idLen};
        offset += sizeof(uint16_t) + *idLen;
        return id;
    }

    uint32_t getUInt(const std::vector<std::byte>& bytes, uint32_t& offset) {
        auto startByte = bytes.begin() + offset;

        std::vector<std::byte> data = {startByte, startByte + sizeof(uint32_t)};

        offset += sizeof(uint32_t);
        return *reinterpret_cast<uint32_t*>(data.data());
    }

    float getFloat(const std::vector<std::byte>& bytes, uint32_t& offset) {
        auto startByte = bytes.begin() + offset;

        std::vector<std::byte> data = {startByte, startByte + sizeof(float)};

        offset += sizeof(uint32_t);
        return *reinterpret_cast<float*>(data.data());
    }
    /// @endcond

    std::pair<VectrixResult, std::shared_ptr<Scene>> Scene::loadScene(SceneCreationData &creationData) {
        if (creationData.result!=SUCCESS) {
            return {creationData.result,nullptr};
        }
        std::shared_ptr<Scene> scene = std::make_shared<Scene>(creationData.name);

        for (const auto& entityData : creationData.entities) {
            auto entity = scene->createEntity(entityData.name);
            for (const auto& componentData : entityData.components) {
                if (entt::type_hash<TransformComponent>::value()==componentData.type_id) {
                    auto& t = entity->getComponent<TransformComponent>();
                    std::memcpy(&t, componentData.data.data(), sizeof(TransformComponent));
                } else if (entt::type_hash<MeshRendererComponent>::value() == componentData.type_id) {
                    auto& m = entity->addComponent<MeshRendererComponent>();
                    uint32_t offset = 0;
                    // Mesh
                    auto resultMesh = AssetsManager::load<Mesh>(getString(componentData.data,offset));
                    if (resultMesh.first!=SUCCESS) {
                        VC_CORE_ERROR_NO_EXIT("Can't load mesh from scene {}: {}",creationData.name,toString(resultMesh.first));
                        return {resultMesh.first,nullptr};
                    }
                    m.mesh = resultMesh.second;
                    // Texture
                    auto resultTexture = AssetsManager::load<Texture>(getString(componentData.data,offset));
                    if (resultTexture.first!=SUCCESS) {
                        VC_CORE_ERROR_NO_EXIT("Can't load texture from scene {}: {}",creationData.name,toString(resultTexture.first));
                        return {resultTexture.first,nullptr};
                    }
                    m.texture = resultTexture.second;
                    // Shader
                    auto resultShader = AssetsManager::load<Shader>(getString(componentData.data,offset));
                    if (resultShader.first!=SUCCESS) {
                        VC_CORE_ERROR_NO_EXIT("Can't load shader from scene {}: {}",creationData.name,toString(resultShader.first));
                        return {resultShader.first,nullptr};
                    }
                    m.shader = resultShader.second;
                    auto startEnableData = componentData.data.begin() + offset;
                    std::vector<std::byte> enableData = {startEnableData,startEnableData + sizeof(bool)};
                    bool* enable = reinterpret_cast<bool*>(enableData.data());
                    if (*enable) {
                        if (!m.tryEnabling()) {
                            VC_CORE_ERROR_NO_EXIT("Can't enable the entity {}, from scene {}",entityData.name,creationData.name);
                            return {UNKNOWN_ERROR,nullptr};
                        }
                    }
                } else if (entt::type_hash<CameraComponent>::value()==componentData.type_id) {
                    auto& c = entity->addComponent<CameraComponent>();
                    uint32_t offset = 0;
                    float fov = getFloat(componentData.data,offset);
                    c.camera.setFOV(fov);

                    float camNear = getFloat(componentData.data,offset);
                    c.camera.setCamNear(camNear);

                    float camFar = getFloat(componentData.data,offset);
                    c.camera.setCamFar(camFar);

                    float aspect = getFloat(componentData.data,offset);
                    if (aspect!=-1) {
                        c.camera.setCustomAspect(aspect);
                    }
                }
            }
        }
        return {SUCCESS, scene};
    }

    Scene::Scene(std::string name) : m_name(std::move(name)) {
        m_entities.clear();
        m_entities.reserve(256);
    }

    Scene::~Scene() = default;

    void Scene::OnUpdate(DeltaTime dt) {

    }

    void Scene::OnRender() {
        auto view = m_registry.view<MeshRendererComponent, TransformComponent>();

        for (auto entity : view) {
            auto& mesh = view.get<MeshRendererComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);

            if (!mesh.isEnable()) continue;

            Renderer::submit(mesh.shader,mesh.mesh->getVertexArray(),transform.modelMatrix(),mesh.shader->useTexture(mesh.texture));
        }
    }

    std::shared_ptr<Entity> Scene::createEntity(const std::string& name) {
        auto *e = new Entity(m_registry.create(), this);
        std::shared_ptr<Entity> entity = std::shared_ptr<Entity>(e);
        m_entities.emplace(entity->getID(), entity);

        auto& info = entity->addComponent<InformationComponent>();
        info.name = name.empty() ? "Entity" : name;
        entity->addComponent<TransformComponent>();
        return entity;
    }

    void Scene::destroyEntity(Entity entity) {
        m_registry.destroy(entity);
    }

} // Vectrix