#ifndef VECTRIXWORKSPACE_MESH_H
#define VECTRIXWORKSPACE_MESH_H
#include <memory>

#include "VertexArray.h"
#include "Vectrix/Physics/AABB.h"

namespace Vectrix {
    class Mesh {
    public:
        Mesh(const std::vector<Vertex>& vertices);
        Mesh(const std::vector<Vertex>& vertices, std::vector<uint32_t> indices);

        [[nodiscard]] std::shared_ptr<VertexArray> getVertexArray() const { return m_vertexArray; }
        [[nodiscard]] AABB& getAABB() { return m_aabb; }
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