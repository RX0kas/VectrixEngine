#ifndef VECTRIXWORKSPACE_CAMERACOMPONENT_H
#define VECTRIXWORKSPACE_CAMERACOMPONENT_H
#include <memory>

#include "Vectrix/Rendering/Camera/Camera.h"


/**
 * @file CameraComponent.h
 * @brief Definition of the component giving an entity a point of view
 * @ingroup ecs
 */

namespace Vectrix {
    /**
     * @brief Gives an entity a camera, so the scene can be rendered from it
     *
     * The camera follows the TransformComponent of the entity it is attached to, so
     * moving the entity moves the point of view.
     * @see Camera
     * @see TransformComponent
     * @ingroup ecs
     */
    struct CameraComponent {
        /// The camera itself, bound to the entity owning the component
        Camera camera;

        /**
         * @brief Copy the component, keeping the same camera settings
         */
        CameraComponent(const CameraComponent&) = default;

        /**
         * @brief Build the component and bind its camera to the entity
         * @param entity The entity the component is added to
         */
        CameraComponent(std::shared_ptr<Entity> entity) : camera(entity) {}
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_CAMERACOMPONENT_H