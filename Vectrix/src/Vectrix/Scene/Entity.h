#ifndef VECTRIXWORKSPACE_ENTITY_H
#define VECTRIXWORKSPACE_ENTITY_H

#include <type_traits>

#include "Components/TransformComponent.h"
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
        Entity() : m_scene(nullptr), m_entityHandle(entt::null){}

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
            return m_scene->m_registry.emplace<T>(m_entityHandle,this,std::forward<Args>(args)...);
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
            bool isTransfComp = std::is_same_v<T, TransformComponent>;
            VC_CORE_ASSERT(!isInfoComp,"Can't remove InformationComponent");
            VC_CORE_ASSERT(!isTransfComp,"Can't remove TransformComponent");
            m_scene->m_registry.erase<T>(m_entityHandle);
        }

        /**
         * @brief This function return the component of the entity from a certain type
         * @pre The entity must have the component
         * @tparam T The component type
         */
        template<typename T>
        T& getComponent() {
            VC_CORE_ASSERT(hasComponent<T>(),"Entity has not component");
            return m_scene->m_registry.get<T>(m_entityHandle);
        }

        /**
         * @brief Checks if the entity is valid
         */
        [[nodiscard]] bool isValid() const {
            return this;
        }

        /**
         * @brief Checks if the entity is valid
         */
        operator bool() const { return m_entityHandle != entt::null; }
        operator entt::entity() const { return m_entityHandle; }
        operator uint32_t() const { return static_cast<uint32_t>(m_entityHandle); }

        bool operator==(const Entity& other) const
        {
            return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
        }

        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }

        static Entity nullEntity() { return {};}

        [[nodiscard]] entt::entity getID() const { return m_entityHandle; }
    private:
        friend class Scene;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;
        entt::entity m_entityHandle{ entt::null };
        Scene* m_scene = nullptr;
    };

} // Vectrix

#endif //VECTRIXWORKSPACE_ENTITY_H