#ifndef VECTRIXWORKSPACE_AABB_H
#define VECTRIXWORKSPACE_AABB_H
#include "Vectrix/Rendering/Mesh/Vertex.h"

namespace Vectrix {
    class AABB {
    public:
        AABB() = default;
        AABB(const std::vector<Vertex> &vertices);

        void update(const std::vector<Vertex> &vertices);

        bool intersect(glm::vec3 rayOrigin, glm::vec3 rayDir,float& tOut) const;
    private:
        glm::vec3 m_min{};
        glm::vec3 m_max{};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_AABB_H