#ifndef VECTRIXWORKSPACE_SCENE_H
#define VECTRIXWORKSPACE_SCENE_H

#include <filesystem>

#include "Vectrix/Utils/Result.h"
#include "SceneSerializer.h"
#include "entt/entt.hpp"
#include "Vectrix/Core/DeltaTime.h"
#include "Vectrix/Utils/Memory.h"
#include "Vectrix/Utils/Path.h"
#include "Vectrix/Assets/AssetFingerprint.h"

#include <unordered_map>

/**
 * @file Scene.h
 * @brief Definition of the Scene class
 * @ingroup ecs
 */

namespace Vectrix {
    class Entity;
    struct SceneCreationData;

    /**
     * @brief Holds every entity of a level, and drives them each frame
     *
     * A scene owns the registry the entities live in, so an entity is only valid as long
     * as its scene is. Create the entities through createEntity rather than building them
     * by hand, that way they get the components every entity is expected to have.
     * @see Entity
     * @see SceneSerializer
     * @ingroup ecs
     */
    class Scene {
    public:
        /**
         * @brief Create an empty scene
         * @param name The name of the scene
         */
        Scene(std::string name);
        ~Scene();

        /**
         * @brief Advance every entity of the scene by one frame
         * @param dt The time elapsed since the last frame
         */
        void OnUpdate(DeltaTime dt);

        /**
         * @brief Draw every entity of the scene that can be rendered
         * @note Called by the layer owning the scene, inside its render callback
         */
        void OnRender();

        /**
         * @brief It creates an entity in the scene with TransformComponent and TagComponent
         * @param name The name of the Entity
         * @return The new entity, owned by the scene
         */
        std::shared_ptr<Entity> createEntity(const std::string& name="");

        /**
         * @brief Remove an entity and everything it owns from the scene
         * @param entity The entity to destroy (an Entity converts to this implicitly)
         * @warning Every reference to that entity becomes invalid
         */
        void destroyEntity(entt::entity entity);

        /**
         * @brief Return the name of the scene
         * @return The scene name
         */
        [[nodiscard]] std::string getName() const { return m_name; }

        /**
         * @brief Return the folder the scene file is in
         * @return The scene's directory (e.g. `<project>/Scenes`), empty until the scene is loaded or saved
         * @note Not the project directory: asset paths are resolved against AssetsManager::getAssetsPath
         */
        [[nodiscard]] std::string getDirectory() const { return toUtf8(m_directory); }

        /**
         * @brief Return the name of the file the scene was loaded from
         * @return The file name, empty when the scene was never saved
         */
        [[nodiscard]] std::string getFileName() const { return m_fileName; }

        /**
         * @brief Build a scene from what was read out of a scene file
         *
         * An asset that can't be loaded (moved, deleted, broken) doesn't stop the scene from loading: the
         * component referring to it keeps its path (MeshRendererComponent::missingMesh, ...), which saving writes
         * back, and is disabled when it needed it to be drawn.
         * @param creationData The content of the file, as returned by SceneSerializer::loadSceneFile
         * @param assetErrors When not null, receives one line per asset left out ("entity: path (reason)")
         * @return Whether it worked, and the scene when it did
         * @see SceneSerializer::loadSceneFile
         */
        static std::pair<VectrixResult,std::shared_ptr<Scene>> loadScene(SceneCreationData& creationData, std::vector<std::string>* assetErrors = nullptr);
    private:
        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;
        friend class SceneSerializer;
        std::shared_ptr<Entity> getEntity(entt::entity entityHandle) {
            const auto it = m_entities.find(entityHandle); // not operator[]: it would insert a null entry on a miss
            return it != m_entities.end() ? it->second : nullptr;
        }
        entt::registry m_registry;
        std::string m_name;
        std::string m_fileName;
        /// Fingerprints read from the scene file for the assets that couldn't be loaded, by id: written back when the
        /// scene is saved, and used to find those assets again if they were moved outside the editor
        std::unordered_map<std::string, AssetFingerprint> m_missingFingerprints;
        std::filesystem::path m_directory;
        std::unordered_map<entt::entity,std::shared_ptr<Entity>> m_entities{};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENE_H
