#ifndef VECTRIXWORKSPACE_MESHREGISTRY_H
#define VECTRIXWORKSPACE_MESHREGISTRY_H
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
    class MeshHandle;

    class MeshRegistry {
    public:
        MeshRegistry();
        ~MeshRegistry();
        MeshHandle registerMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        void uploadToGPU();

        VulkanBuffer& getVertexBuffer() const { return *m_globalVertexBuffer; }
        VulkanBuffer& getIndexBuffer() const { return *m_globalIndexBuffer;  }
    private:
        void uploadBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, std::unique_ptr<VulkanBuffer>& outBuffer);


        std::vector<Vertex>   m_pendingVertices;
        std::vector<uint32_t> m_pendingIndices;

        Own<VulkanBuffer> m_globalVertexBuffer;
        Own<VulkanBuffer> m_globalIndexBuffer;
        bool m_uploaded;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHREGISTRY_H