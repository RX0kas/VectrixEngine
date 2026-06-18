#include "AABB.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/component_wise.hpp>

#include "Vectrix/Core/Log.h"

namespace Vectrix {
    AABB::AABB(const std::vector<Vertex> &vertices) {
        update(vertices);
    }

    void AABB::update(const std::vector<Vertex> &vertices) {
        if (vertices.empty()) {
            VC_CORE_WARN("Can't update the AABB because the vertices are empty");
            m_min = {0,0,0};
            m_max = {0,0,0};
            return;
        }

        m_min = vertices[0].position;
        m_max = vertices[0].position;

        for (const auto& v : vertices) {
            if (v.position.x < m_min.x) m_min.x = v.position.x;
            if (v.position.y < m_min.y) m_min.y = v.position.y;
            if (v.position.z < m_min.z) m_min.z = v.position.z;

            if (v.position.x > m_max.x) m_max.x = v.position.x;
            if (v.position.y > m_max.y) m_max.y = v.position.y;
            if (v.position.z > m_max.z) m_max.z = v.position.z;
        }
    }

    bool AABB::intersect(glm::vec3 rayOrigin, glm::vec3 rayDir, float& tOut) const {
        glm::vec3 invDir = 1.0f / rayDir;
        glm::vec3 t1 = (m_min - rayOrigin) * invDir;
        glm::vec3 t2 = (m_max - rayOrigin) * invDir;

        float tMin = glm::compMax(glm::min(t1, t2));
        float tMax = glm::compMin(glm::max(t1, t2));

        tOut = tMin;

        return tMax >= tMin && tMax > 0.0f;
    }
} // Vectrix