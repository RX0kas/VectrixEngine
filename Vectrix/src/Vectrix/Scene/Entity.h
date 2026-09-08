#ifndef VECTRIXWORKSPACE_ENTITY_H
#define VECTRIXWORKSPACE_ENTITY_H

#include <type_traits>

#include "Component.h"
#include "Vectrix/Scene/Scene.h"
#include "Vectrix/Core/Core.h"
#include "Vectrix/Core/Log.h"

/**
 * @file Entity.h
 * @brief Definition of the Entity class
 * @ingroup ecs
 */


namespace Vectrix {
    /**
     * @brief A handle on one object of a Scene, and on the components it owns
     *
     * An entity is a lightweight handle rather than a container: it holds which scene it
     * belongs to and an identifier, the components themselves live in the scene. Copying
     * one is cheap and gives another handle on the same object. Build them with
     * Scene::createEntity.
     * @see Scene::createEntity
     * @see InformationComponent
     * @ingroup ecs
     */
    class Entity {
    public:
        /**
         * @brief Build a handle pointing at nothing
         * @see nullEntity
         * @see isValid
         */
        Entity() : m_scene(nullptr), m_entityHandle(entt::null){}

        /**
         * @brief This function add a component to the entity
         * @pre The entity must not have the component
         * @tparam T The component type
         * @tparam Args The constructor type
         * @param args Constructor arguments
         * @return A reference to the component that was just added
         */
        template<typename T,typename... Args>
        T& addComponent(Args&&... args) {
            VC_CORE_ASSERT(!hasComponent<T>(), "Entity already has component");
            return m_scene->m_registry.emplace<T>(m_entityHandle,m_scene->getEntity(m_entityHandle),std::forward<Args>(args)...);
        }

        /**
         * @brief This function tell if the entity has the component
         * @tparam T The component type
         * @return true when the entity owns a component of that type
         */
        template<typename T>
        [[nodiscard]] bool hasComponent() const {
            return m_scene->m_registry.all_of<T>(m_entityHandle);
        }

        /**
         * @brief This function remove a component from the entity
         * @pre The entity must have the component
         * @tparam T The component type
         * @warning InformationComponent and TransformComponent cannot be removed, every
         * entity is expected to have them
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
         * @return A reference to the component, valid while the entity keeps it
         * @see hasComponent
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
         * @return true when the handle points at an actual entity
         */
        operator bool() const { return m_entityHandle != entt::null; }

        /**
         * @brief Convert to the underlying registry handle
         * @return The handle the entity wraps
         */
        operator entt::entity() const { return m_entityHandle; }

        /**
         * @brief Convert to a plain number, mostly to use as an identifier in the UI
         * @return The handle as an unsigned integer
         */
        operator uint32_t() const { return static_cast<uint32_t>(m_entityHandle); }

        /**
         * @brief Tell if two handles point at the same entity of the same scene
         * @param other The entity to compare with
         * @return true when both describe the same object
         */
        bool operator==(const Entity& other) const
        {
            return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
        }

        /**
         * @brief Tell if two handles point at different entities
         * @param other The entity to compare with
         * @return true when they do not describe the same object
         */
        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }

        /**
         * @brief Return a handle pointing at nothing
         * @return An entity that converts to false
         */
        static Entity nullEntity() { return {};}

        /**
         * @brief Return the identifier of the entity inside its scene
         * @return The registry handle
         */
        [[nodiscard]] entt::entity getID() const { return m_entityHandle; }
    private:
        friend class Scene;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;
        friend class SceneSerializer;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;
        entt::entity m_entityHandle{ entt::null };
        Scene* m_scene = nullptr;
    };

} // Vectrix

#endif //VECTRIXWORKSPACE_ENTITY_H