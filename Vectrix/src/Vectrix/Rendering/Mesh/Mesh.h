#ifndef VECTRIXWORKSPACE_MESH_H
#define VECTRIXWORKSPACE_MESH_H
#include <memory>

#include "VertexArray.h"
#include "Vectrix/Physics/AABB.h"

/**
 * @file Mesh.h
 * @brief Definition of the Mesh class
 * @ingroup mesh
 */

namespace Vectrix {
    /**
     * @brief The geometry of an object, ready to be drawn
     *
     * A mesh owns the vertices on the GPU and the bounding box around them. Rather than
     * building one by hand, load it through AssetsManager::load so the same file is not
     * uploaded twice.
     * @see AssetsManager::load
     * @see MeshRendererComponent
     * @ingroup mesh
     */
    class Mesh {
    public:
        /**
         * @brief Build a mesh from vertices alone, drawing them in order
         * @param vertices The vertices making up the geometry
         */
        Mesh(const std::vector<Vertex>& vertices);

        /**
         * @brief Build a mesh from vertices and the indices pointing into them
         *
         * Indexing lets a vertex shared by several triangles be stored once.
         * @param vertices The vertices making up the geometry
         * @param indices Which vertices each triangle uses
         */
        Mesh(const std::vector<Vertex>& vertices, std::vector<uint32_t> indices);

        /**
         * @brief Return the GPU buffers holding the geometry
         * @return The vertex array of the mesh
         */
        [[nodiscard]] std::shared_ptr<VertexArray> getVertexArray() const { return m_vertexArray; }

        /**
         * @brief Return the box around the geometry, used for picking
         * @return The bounding box, in the local space of the mesh
         * @see AABB
         */
        [[nodiscard]] AABB& getAABB() { return m_aabb; }

        /**
         * @brief Return the name the mesh is known by
         * @return The mesh identifier, `"None"` when it was never registered
         */
        [[nodiscard]] std::string getID() const { return m_id; }
    private:
        friend class AssetsManager;
        friend class MeshManager;
        friend class VulkanContext;
        friend class Scene;
        void registerMesh();
        std::shared_ptr<VertexArray> m_vertexArray;
        BufferLayout m_layout;
        AABB m_aabb;
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::string m_id = "None";
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESH_H