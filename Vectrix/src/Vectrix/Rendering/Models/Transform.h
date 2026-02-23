#ifndef VECTRIXWORKSPACE_TRANSFORM_H
#define VECTRIXWORKSPACE_TRANSFORM_H

#include <glm/gtc/matrix_transform.hpp>

namespace Vectrix {
    class Transform {
    public:
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_TRANSFORM_H