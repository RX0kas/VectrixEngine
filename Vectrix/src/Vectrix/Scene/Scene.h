#ifndef VECTRIXWORKSPACE_SCENE_H
#define VECTRIXWORKSPACE_SCENE_H

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

    class Scene {
    public:
        Scene(std::string name);
        ~Scene();

        void OnUpdate(DeltaTime dt);
        void OnRender();

        /**
         * @brief It creates an entity in the scene with TransformComponent and TagComponent
         * @param name The name of the Entity
         */
        std::shared_ptr<Entity> createEntity(const std::string& name="");

        void destroyEntity(Entity entity);
        [[nodiscard]] std::string getName() const { return m_name; }
        [[nodiscard]] std::string getFilePath() const { return m_filePath; }

        static std::pair<VectrixResult,std::shared_ptr<Scene>> loadScene(SceneCreationData& creationData);
        void registerAllMesh();
    private:
        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;
        friend class SceneSerializer;
        friend class MenuBar;
        std::shared_ptr<Entity> getEntity(entt::entity entityHandle) { return m_entities[entityHandle]; }
        entt::registry m_registry;
        std::string m_name;
        std::string m_filePath;
        std::unordered_map<entt::entity,std::shared_ptr<Entity>> m_entities{};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENE_H