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
        std::shared_ptr<Shader> shader;
        bool enable = true;

        MeshComponent(const MeshComponent&) = delete;
        MeshComponent(const std::vector<Vertex>& vertices, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture);
        MeshComponent(const std::string &pathObj, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture);
        MeshComponent(const std::vector<Vertex>& vertices, std::vector<uint32_t> indices, std::shared_ptr<Shader> shader, const std::shared_ptr<Texture>& texture);
    private:
        void registerMesh();
        friend class VulkanContext;
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHCOMPONENT_H