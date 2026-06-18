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
     */
    class TransformComponent {
    public:
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
         **/
        [[nodiscard]] glm::vec3 getRotationDeg() const {
            return glm::degrees(glm::eulerAngles(rotation));
        }

    };
} // Vectrix
#endif //VECTRIXWORKSPACE_TRANSFORMCOMPONENT_H