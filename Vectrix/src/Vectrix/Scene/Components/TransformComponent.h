#ifndef VECTRIXWORKSPACE_TRANSFORMCOMPONENT_H
#define VECTRIXWORKSPACE_TRANSFORMCOMPONENT_H

#include <memory>
#include <glm/gtc/matrix_transform.hpp>

#include "Vectrix/Utils/Math.h"

/**
 * @file TransformComponent.h
 * @brief Definition of the Transform class
 * @ingroup mesh
 */

namespace Vectrix {
    class Entity;
    /**
     * @brief This class represent the 3D information about an object
     *
     * Every entity has one, and it is what places the entity in the world. The rotation
     * is kept as a quaternion, use setRotationDeg and getRotationDeg to work in degrees.
     * @ingroup ecs
     */
    class TransformComponent {
    public:
        /**
         * @brief Build a transform at the origin, unscaled and unrotated
         * @param entity The entity the component is added to
         */
        TransformComponent(std::shared_ptr<Entity> entity) {}
        /**
         * @brief This is the world position
         */
        glm::vec3 position{ 0};
        /**
         * @brief This is the scale or the size
         */
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        /**
         * @brief This is the rotation (quaternion)
         */
        glm::quat rotation = glm::identity<glm::quat>();

        /**
        * @brief This function return the model matrix
        * @details Matrix corresponds to Translate * Ry * Rx * Rz * Scale
        * Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        * https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        * @return The model matrix of an object
        */
        [[nodiscard]] glm::mat4 modelMatrix() const;

        /**
         * @brief Build a model matrix out of loose values, without needing a component
         * @param pos The world position
         * @param scale The scale on each axis
         * @param rotation The rotation
         * @return The matching model matrix
         */
        static glm::mat4 modelMatrix(glm::vec3 pos, glm::vec3 scale, glm::quat rotation);

        /**
        * @brief This function return a matrix that is the equivalent of "glm::transpose(glm::inverse(modelMatrix))"
        * @return The normal matrix of an object
        */
        [[nodiscard]] glm::mat3 normalMatrix() const;

        /**
         * @brief Sets the rotation of the model in degrees
         * @param newRotation Euler angles in degrees
         */
        void setRotationDeg(glm::vec3 newRotation) {
            rotation = glm::quat(glm::radians(newRotation));
        }


        /**
         * @brief Returns the rotation of the model in degrees
         * @return The rotation as Euler angles in degrees
         * @see setRotationDeg
         **/
        [[nodiscard]] glm::vec3 getRotationDeg() const {
            return glm::degrees(glm::eulerAngles(rotation));
        }

    };
} // Vectrix
#endif //VECTRIXWORKSPACE_TRANSFORMCOMPONENT_H