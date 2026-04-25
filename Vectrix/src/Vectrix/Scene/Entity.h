#ifndef VECTRIXWORKSPACE_ENTITY_H
#define VECTRIXWORKSPACE_ENTITY_H

#include <type_traits>
#include "entt/entt.hpp"
#include "Vectrix/Scene/Scene.h"
#include "Vectrix/Core/Core.h"
#include "Vectrix/Core/Log.h"

/**
 * @file Entity.h
 * @brief Definition of the Entity class
 * @ingroup ecs
 */

struct InformationComponent;

namespace Vectrix {
    class Entity {
    public:
        /**
         * @brief This function add a component to the entity
         * @pre The entity must not have the component
         * @tparam T The component type
         * @tparam Args The constructor type
         * @param args Constructor arguments
         */
        template<typename T,typename... Args>
        T& addComponent(Args&&... args) {
            VC_CORE_ASSERT(!hasComponent<T>(), "Entity already has component");
            return m_scene->m_registry.emplace<T>(m_entityHandle,std::forward<Args>(args)...);
        }

        /**
         * @brief This function tell if the entity has the component
         * @tparam T The component type
         */
        template<typename T>
        [[nodiscard]] bool hasComponent() const {
            return m_scene->m_registry.all_of<T>(m_entityHandle);
        }

        /**
         * @brief This function remove a component from the entity
         * @pre The entity must have the component
         * @tparam T The component type
         */
        template<typename T>
        void deleteComponent() {
            VC_CORE_ASSERT(hasComponent<T>(), "Entity has not component");
            bool isInfoComp = std::is_same_v<T, InformationComponent>;
            VC_CORE_ASSERT(!isInfoComp,"Can't remove InformationComponent");
            m_scene->m_registry.erase<T>(m_entityHandle);
        }

        /**
         * @brief This function return the component of the entity from a certain type
         * @pre The entity must have the component
         * @tparam T The component type
         */
        template<typename T>
        T getComponent() {
            VC_CORE_ASSERT(hasComponent<T>(),"Entity has not component");
            return m_scene->m_registry.get<T>(m_entityHandle);
        }

        /**
         * @brief Checks if the entity is valid
         */
        explicit operator bool() const { return m_scene->m_registry.valid(m_entityHandle) && m_entityHandle!=entt::null; }

        static Entity nullEntity() { return Entity();}
    private:
        friend class Scene;
        Entity(entt::entity handle, Scene* scene);
        Entity() = default;
        Entity(const Entity& other) = default;
        entt::entity m_entityHandle{ entt::null };
        Scene* m_scene = nullptr;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_ENTITY_H