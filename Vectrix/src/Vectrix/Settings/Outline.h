#ifndef VECTRIXWORKSPACE_OUTLINE_H
#define VECTRIXWORKSPACE_OUTLINE_H
#include <glm/glm.hpp>

namespace Vectrix {
    struct OutlineSettings {
        glm::vec4 color = {1.0f,1.0f,1.0f,1.0f};
        float thickness = 0.1f;
        glm::vec2 texelSize = {1.0f, 1.0f};
    };

    inline OutlineSettings outlineSettings;
}

#endif //VECTRIXWORKSPACE_OUTLINE_H
