#ifndef VECTRIXWORKSPACE_TRANSFORM_H
#define VECTRIXWORKSPACE_TRANSFORM_H

#include <glm/gtc/matrix_transform.hpp>

/**
 * @file Transform.h
 * @brief Definition of the Transform class
 * @ingroup mesh
 */

namespace Vectrix {
    /**
     * @brief This class represent the 3D information about an object
     */
    class Transform {
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
        * @deprecated Please use Transform::modelMatrix() instead
        * @return The model matrix of an object
        */
        [[nodiscard]] glm::mat4 mat4() const { return modelMatrix(); }
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
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_TRANSFORM_H