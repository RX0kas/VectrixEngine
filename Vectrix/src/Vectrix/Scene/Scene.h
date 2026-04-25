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

        Entity createEntity(const std::string& name="");
    private:
        friend class Entity;
        entt::registry m_registry;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENE_H