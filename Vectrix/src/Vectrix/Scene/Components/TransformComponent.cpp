#include "TransformComponent.h"

namespace Vectrix {
    glm::mat4 TransformComponent::modelMatrix() const {
        return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
    }

    glm::mat3 TransformComponent::normalMatrix() const {
        return glm::transpose(glm::inverse(modelMatrix()));
    }
} // Vectrix