#include "Scene.h"

#include <algorithm>
#include <format>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Component.h"
#include "Entity.h"
#include "Vectrix/Assets/AssetsManager.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    namespace {
        /// Reads the serialized fields of one component, refusing to read past its end (a truncated or
        /// corrupt scene file must fail to load, not read out of bounds)
        class ComponentReader {
        public:
            explicit ComponentReader(const std::vector<std::byte>& bytes) : m_bytes(bytes) {}

            bool read(void* out, const size_t size) {
                if (m_bytes.size() - m_offset < size) return false;
                std::memcpy(out, m_bytes.data() + m_offset, size);
                m_offset += size;
                return true;
            }

            bool readFloat(float& out) { return read(&out, sizeof(float)); }

            bool readBool(bool& out) {
                std::uint8_t byte = 0; // read as a byte: a value other than 0/1 read straight into a bool is UB
                if (!read(&byte, sizeof(byte))) return false;
                out = byte != 0;
                return true;
            }

            bool readString(std::string& out) {
                std::uint16_t length = 0;
                if (!read(&length, sizeof(length))) return false;
                if (m_bytes.size() - m_offset < length) return false;
                out.assign(reinterpret_cast<const char*>(m_bytes.data() + m_offset), length);
                m_offset += length;
                return true;
            }

        private:
            const std::vector<std::byte>& m_bytes;
            size_t m_offset = 0;
        };

        std::pair<VectrixResult, std::shared_ptr<Scene>> malformed(const std::string& sceneName, const std::string& entityName) {
            VC_CORE_ERROR_NO_EXIT("Malformed component data for entity {} in scene {}", entityName, sceneName);
            return {FORMATING_ERROR, nullptr};
        }

        /// The scene's assets, loaded together on the worker threads before the entities are built, by
        /// "<AssetType>:<id>"
        using PreloadedAssets = std::unordered_map<std::string, std::pair<VectrixResult, std::shared_ptr<void>>>;

        std::string preloadKey(const AssetType type, const std::string& id) {
            return std::to_string(static_cast<int>(type)) + ":" + id;
        }

        /// Start loading every asset the scene's components refer to, then wait for them all: the files are
        /// read, decoded, uploaded and compiled at the same time instead of one after the other
        PreloadedAssets preloadSceneAssets(const SceneCreationData& creationData) {
            VC_PROFILER_FUNCTION();
            std::vector<AssetLoad<Mesh>> meshes;
            std::vector<AssetLoad<Texture>> textures;
            std::vector<AssetLoad<Shader>> shaders;
            for (const auto& entityData : creationData.entities) {
                for (const auto& componentData : entityData.components) {
                    if (entt::type_hash<MeshRendererComponent>::value() != componentData.type_id)
                        continue;
                    ComponentReader reader(componentData.data);
                    std::string meshId, textureId, shaderId;
                    if (!reader.readString(meshId) || !reader.readString(textureId) || !reader.readString(shaderId))
                        continue; // Reported as malformed when the entities are built
                    for (std::string* id : {&meshId, &textureId, &shaderId})
                        std::ranges::replace(*id, '\\', '/');
                    if (!meshId.empty()) meshes.push_back(AssetsManager::loadAsync<Mesh>(meshId));
                    if (!textureId.empty()) textures.push_back(AssetsManager::loadAsync<Texture>(textureId));
                    if (!shaderId.empty()) shaders.push_back(AssetsManager::loadAsync<Shader>(shaderId));
                }
            }

            PreloadedAssets preloaded;
            const auto collect = [&preloaded](const auto& loads, const AssetType type) {
                for (const auto& load : loads) {
                    auto [result, asset] = load.wait();
                    preloaded[preloadKey(type, load.getId())] = {result, std::move(asset)};
                }
            };
            collect(meshes, MESH);
            collect(textures, TEXTURE);
            collect(shaders, SHADER);
            return preloaded;
        }

        /// Loads the asset a MeshRendererComponent refers to. An empty id is an asset that was never set
        /// (the component was saved half configured): it stays null. An asset that can't be loaded is reported
        /// and its id kept in missingId, so saving writes the reference back: one moved or deleted file must
        /// neither make the whole scene unopenable nor be dropped from it
        template<typename T>
        void loadComponentAsset(std::string id, std::shared_ptr<T>& out, std::string& missingId, const std::string& entityName,
                                const std::string& sceneName, std::vector<std::string>* assetErrors, const PreloadedAssets& preloaded) {
            if (id.empty()) return;
            // Scenes saved on Windows before ids were normalised hold '\' separators, a plain character elsewhere
            std::ranges::replace(id, '\\', '/');
            // Loaded by preloadSceneAssets in most cases, under the id the path resolves to
            VectrixResult result;
            std::shared_ptr<T> asset;
            const std::optional<std::string> resolvedId = AssetsManager::getProjectAssetId(fromUtf8(id));
            if (const auto it = preloaded.find(preloadKey(AssetsManager::assetTypeOf<T>(), resolvedId.value_or(id))); it != preloaded.end()) {
                result = it->second.first;
                asset = std::static_pointer_cast<T>(it->second.second);
            } else {
                std::tie(result, asset) = AssetsManager::load<T>(id);
            }
            if (result != SUCCESS) {
                VC_CORE_WARN("Can't load {} for entity {} of scene {}: {}", id, entityName, sceneName, toString(result));
                if (assetErrors)
                    assetErrors->push_back(std::format("{}: {} ({})", entityName, id, toString(result)));
                missingId = std::move(id);
                // A texture has a stand-in that shows something is missing; a mesh or shader doesn't
                if constexpr (std::is_same_v<T, Texture>)
                    out = TextureManager::getNotFoundTexture();
                return;
            }
            out = asset;
        }
    }

    std::pair<VectrixResult, std::shared_ptr<Scene>> Scene::loadScene(SceneCreationData &creationData, std::vector<std::string>* assetErrors) {
        if (creationData.result!=SUCCESS) {
            return {creationData.result,nullptr};
        }
        std::shared_ptr<Scene> scene = std::make_shared<Scene>(creationData.name);
        const PreloadedAssets preloaded = preloadSceneAssets(creationData);

        for (const auto& entityData : creationData.entities) {
            auto entity = scene->createEntity(entityData.name);
            for (const auto& componentData : entityData.components) {
                ComponentReader reader(componentData.data);

                if (entt::type_hash<TransformComponent>::value()==componentData.type_id) {
                    if (componentData.data.size() != sizeof(TransformComponent))
                        return malformed(creationData.name, entityData.name);
                    auto& t = entity->getComponent<TransformComponent>();
                    std::memcpy(&t, componentData.data.data(), sizeof(TransformComponent));
                } else if (entt::type_hash<MeshRendererComponent>::value() == componentData.type_id) {
                    std::string meshId, textureId, shaderId;
                    bool enable = false;
                    if (!reader.readString(meshId) || !reader.readString(textureId) || !reader.readString(shaderId) || !reader.readBool(enable))
                        return malformed(creationData.name, entityData.name);

                    // Each asset's fingerprint follows in scenes saved since they exist (older ones end with the flag)
                    std::array<AssetFingerprint, 3> fingerprints{};
                    for (AssetFingerprint& fingerprint : fingerprints) {
                        if (!reader.read(&fingerprint.size, sizeof(fingerprint.size)) || !reader.read(&fingerprint.hash, sizeof(fingerprint.hash))) {
                            fingerprints = {};
                            break;
                        }
                    }

                    auto& m = entity->addComponent<MeshRendererComponent>();
                    loadComponentAsset(meshId, m.mesh, m.missingMesh, entityData.name, creationData.name, assetErrors, preloaded);
                    loadComponentAsset(textureId, m.texture, m.missingTexture, entityData.name, creationData.name, assetErrors, preloaded);
                    loadComponentAsset(shaderId, m.shader, m.missingShader, entityData.name, creationData.name, assetErrors, preloaded);

                    // Kept for the missing ones: saved back, and what finds them again if they were moved
                    const std::array<const std::string*, 3> missing = {&m.missingMesh, &m.missingTexture, &m.missingShader};
                    for (size_t i = 0; i < missing.size(); ++i) {
                        if (!missing[i]->empty() && fingerprints[i].isKnown())
                            scene->m_missingFingerprints[*missing[i]] = fingerprints[i];
                    }

                    // Only fails when an asset it needs was left out above, which is already reported
                    if (enable && !m.tryEnabling()) {
                        m.enabledOnceComplete = true;
                        VC_CORE_WARN("Entity {} of scene {} left disabled: an asset it needs is missing", entityData.name, creationData.name);
                    }
                } else if (entt::type_hash<CameraComponent>::value()==componentData.type_id) {
                    float fov = 0.0f, camNear = 0.0f, camFar = 0.0f, aspect = -1.0f;
                    if (!reader.readFloat(fov) || !reader.readFloat(camNear) || !reader.readFloat(camFar) || !reader.readFloat(aspect))
                        return malformed(creationData.name, entityData.name);

                    auto& c = entity->addComponent<CameraComponent>();
                    c.camera.setFOV(fov);
                    c.camera.setCamNear(camNear);
                    c.camera.setCamFar(camFar);
                    if (aspect > 0.0f) {
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

    Scene::~Scene() {
        // Camera's current camera is static and would otherwise keep an Entity pointing at this destroyed scene
        if (const std::shared_ptr<Entity> current = Camera::getCurrentCamera(); current && current->m_scene == this)
            Camera::clearCurrent();
    }

    void Scene::OnUpdate(DeltaTime dt) {

    }

    void Scene::OnFixedUpdate(DeltaTime fixedDt) {

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

    void Scene::destroyEntity(entt::entity entity) {
        m_entities.erase(entity);
        m_registry.destroy(entity);
    }

} // Vectrix