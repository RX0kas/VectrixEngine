#ifndef VECTRIXWORKSPACE_COMPONENT_H
#define VECTRIXWORKSPACE_COMPONENT_H
#include <memory>
#include <string>
#include <utility>

/**
 * @file Component.h
 * @brief Definition of InformationComponent, and umbrella include of every component
 * @ingroup ecs
 *
 * Including this header brings in every component the engine ships with, so an
 * application only has to include this one.
 */

namespace Vectrix {
    class Entity;

    /**
     * @brief This component is a mandatory component of all entity
     *
     * It carries what identifies an entity for a human, which is what the editor shows in
     * the hierarchy. Scene::createEntity adds it, and Entity::deleteComponent refuses to
     * take it away.
     * @see Entity
     * @ingroup ecs
     */
    struct InformationComponent {
        /// The name the entity is shown under
        std::string name;

        /**
         * @brief Build the component with an empty name
         * @param entity The entity the component is added to
         */
        InformationComponent(std::shared_ptr<Entity> entity) {}

        /**
         * @brief Copy the component, keeping the same name
         */
        InformationComponent(const InformationComponent&) = default;

        /**
         * @brief Build the component with a name
         * @param entity The entity the component is added to
         * @param name The name the entity is shown under
         */
        InformationComponent(std::shared_ptr<Entity> entity, std::string name) : name(std::move(name)) {}
    };
}

#include "Components/TransformComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/CameraComponent.h"

#endif //VECTRIXWORKSPACE_COMPONENT_H