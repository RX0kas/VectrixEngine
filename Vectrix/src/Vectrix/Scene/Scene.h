#ifndef VECTRIXWORKSPACE_SCENE_H
#define VECTRIXWORKSPACE_SCENE_H

#include "entt/entt.hpp"
#include "Vectrix/Core/DeltaTime.h"

/**
 * @file Scene.h
 * @brief Definition of the Scene class
 * @ingroup ecs
 */

namespace Vectrix {
    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        void OnUpdate(DeltaTime dt);
        void OnRender();

        /**
         * @brief It creates an entity in the scene with TransformComponent and TagComponent
         * @param name The name of the Entity
         */
        Entity createEntity(const std::string& name="");

        void destroyEntity(Entity entity);
    private:
        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;
        entt::registry m_registry;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENE_H