#ifndef VECTRIXWORKSPACE_MESHREGISTRY_H
#define VECTRIXWORKSPACE_MESHREGISTRY_H
#include <memory>
#include <vector>

#include "GraphicAPI/Vulkan/Rendering/Data/VulkanBuffer.h"
#include "Vectrix/Rendering/Mesh/Vertex.h"

namespace Vectrix {
    class MeshHandle;

    class MeshRegistry {
    public:
        MeshRegistry();
        ~MeshRegistry();
        MeshHandle registerMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        void uploadToGPU();

        [[nodiscard]] VulkanBuffer& getVertexBuffer() const { return *m_globalVertexBuffer; }
        [[nodiscard]] VulkanBuffer& getIndexBuffer() const { return *m_globalIndexBuffer;  }
        [[nodiscard]] bool isUploaded() const { return m_uploaded; }
    private:
        static void uploadBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, std::unique_ptr<VulkanBuffer>& outBuffer);


        std::vector<Vertex> m_pendingVertices;
        std::vector<uint32_t> m_pendingIndices;

        std::unique_ptr<VulkanBuffer> m_globalVertexBuffer;
        std::unique_ptr<VulkanBuffer> m_globalIndexBuffer;
        bool m_uploaded;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHREGISTRY_H