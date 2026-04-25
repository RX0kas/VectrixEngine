#ifndef VECTRIXWORKSPACE_TRANSFORMCOMPONENT_H
#define VECTRIXWORKSPACE_TRANSFORMCOMPONENT_H

#include <glm/gtc/matrix_transform.hpp>

#include "Vectrix/Utils/Math.h"

/**
 * @file TransformComponent.h
 * @brief Definition of the Transform class
 * @ingroup mesh
 */

namespace Vectrix {
    /**
     * @brief This class represent the 3D information about an object
     */
    class TransformComponent {
    public:
        /**
         * @brief This is the world position
         */
        glm::vec3 translation{};
        /**
         * @brief This is the scale or the size
         */
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        /**
         * @brief This is the rotation (radians)
         */
        glm::vec3 rotation{};

        /**
        * @brief This function return the model matrix
        * @details Matrix corresponds to Translate * Ry * Rx * Rz * Scale
        * Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        * https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        * @return The model matrix of an object
        */
        [[nodiscard]] glm::mat4 modelMatrix() const;

        /**
        * @brief This function return a matrix that is the equivalent of "glm::transpose(glm::inverse(modelMatrix))"
        * @return The normal matrix of an object
        */
        [[nodiscard]] glm::mat3 normalMatrix() const;

        /**
         * @brief Sets the rotation of the model in degrees
         * @param newRotation Euler angles in degrees
         */
        void setRotationDeg(const glm::vec3 newRotation) {
            rotation.x=rad(newRotation.x);
            rotation.y=rad(newRotation.y);
            rotation.z=rad(newRotation.z);
        }

        /**
         * @brief Returns the rotation of the model in degrees
         **/
        [[nodiscard]] glm::vec3 getRotationDeg() const {
            return {deg(rotation.x),deg(rotation.y),deg(rotation.z)};
        }

    };
} // Vectrix
#endif //VECTRIXWORKSPACE_TRANSFORMCOMPONENT_H