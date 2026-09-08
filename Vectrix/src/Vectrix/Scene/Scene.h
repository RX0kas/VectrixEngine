#ifndef VECTRIXWORKSPACE_SCENE_H
#define VECTRIXWORKSPACE_SCENE_H

#include <filesystem>

#include "Vectrix/Utils/Result.h"
#include "SceneSerializer.h"
#include "entt/entt.hpp"
#include "Vectrix/Core/DeltaTime.h"
#include "Vectrix/Utils/Memory.h"

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
         * @param entity The entity to destroy
         * @warning Every reference to that entity becomes invalid
         */
        void destroyEntity(Entity entity);

        /**
         * @brief Return the name of the scene
         * @return The scene name
         */
        [[nodiscard]] std::string getName() const { return m_name; }

        /**
         * @brief Return the folder of the project the scene belongs to
         * @return The project directory, which the asset paths are relative to
         */
        [[nodiscard]] std::string getProjectDirectory() const { return m_projectDirectory; }

        /**
         * @brief Return the name of the file the scene was loaded from
         * @return The file name, empty when the scene was never saved
         */
        [[nodiscard]] std::string getFileName() const { return m_fileName; }

        /**
         * @brief Build a scene from what was read out of a scene file
         * @param creationData The content of the file, as returned by SceneSerializer::loadSceneFile
         * @return Whether it worked, and the scene when it did
         * @see SceneSerializer::loadSceneFile
         */
        static std::pair<VectrixResult,std::shared_ptr<Scene>> loadScene(SceneCreationData& creationData);
    private:
        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;
        friend class SceneSerializer;
        friend class MenuBar;
        std::shared_ptr<Entity> getEntity(entt::entity entityHandle) { return m_entities[entityHandle]; }
        entt::registry m_registry;
        std::string m_name;
        std::string m_fileName;
        std::filesystem::path m_projectDirectory;
        std::unordered_map<entt::entity,std::shared_ptr<Entity>> m_entities{};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENE_H
