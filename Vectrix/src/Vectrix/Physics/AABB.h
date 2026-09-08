#ifndef VECTRIXWORKSPACE_AABB_H
#define VECTRIXWORKSPACE_AABB_H
#include "Vectrix/Rendering/Mesh/Vertex.h"

/**
 * @file AABB.h
 * @brief Definition of the axis aligned bounding box
 * @ingroup physics
 */

namespace Vectrix {
    /**
     * @brief The smallest box, aligned on the world axes, holding a set of vertices
     *
     * It is what the editor tests a ray against to work out which object was clicked,
     * since intersecting a box is far cheaper than intersecting every triangle.
     * @see Mesh
     * @ingroup physics
     */
    class AABB {
    public:
        /**
         * @brief Build an empty box
         * @note Call update before using it
         */
        AABB() = default;

        /**
         * @brief Build the box around a set of vertices
         * @param vertices The vertices the box has to contain
         */
        AABB(const std::vector<Vertex> &vertices);

        /**
         * @brief Recompute the box around a new set of vertices
         * @param vertices The vertices the box has to contain
         * @post The box contains every given vertex and nothing more
         */
        void update(const std::vector<Vertex> &vertices);

        /**
         * @brief Test whether a ray goes through the box
         * @param rayOrigin Where the ray starts, in world space
         * @param rayDir Which way the ray points, in world space
         * @param tOut Set to how far along the ray the box is hit, only when it is hit
         * @return true when the ray enters the box
         */
        bool intersect(glm::vec3 rayOrigin, glm::vec3 rayDir,float& tOut) const;
    private:
        glm::vec3 m_min{};
        glm::vec3 m_max{};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_AABB_H