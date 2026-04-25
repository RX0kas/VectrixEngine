#ifndef VECTRIXWORKSPACE_MESHCOMPONENT_H
#define VECTRIXWORKSPACE_MESHCOMPONENT_H
#include "Vectrix/Rendering/Buffer.h"
#include "Vectrix/Rendering/Mesh/Vertex.h"
#include "Vectrix/Rendering/Mesh/VertexArray.h"
#include "Vectrix/Rendering/Shaders/Shader.h"

namespace Vectrix {
    class MeshComponent {
    public:
        std::shared_ptr<VertexArray> vertexArray;
        std::shared_ptr<Texture> texture;
        BufferLayout layout;
        Shader& shader;
        bool enable = true;

        MeshComponent(const MeshComponent&) = default;
        MeshComponent(const std::vector<Vertex>& vertices, Shader& shader, const std::shared_ptr<Texture>& texture);
        MeshComponent(const std::string &pathObj, Shader& shader, const std::shared_ptr<Texture>& texture);
        MeshComponent(const std::vector<Vertex>& vertices, std::vector<uint32_t> indices, Shader& shader, const std::shared_ptr<Texture>& texture);
    private:
        void registerMesh();
        friend class VulkanContext;
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHCOMPONENT_H