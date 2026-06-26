#ifndef VECTRIXWORKSPACE_MESHREGISTRY_H
#define VECTRIXWORKSPACE_MESHREGISTRY_H
#include <memory>
#include <vector>

#include "GraphicAPI/Vulkan/Rendering/Data/VulkanBuffer.h"
#include "Vectrix/Rendering/Mesh/Vertex.h"

namespace Vectrix {
    struct MeshHandle;

    class MeshRegistry {
    public:
        MeshRegistry();
        ~MeshRegistry();
        MeshHandle uploadMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        [[nodiscard]] VulkanBuffer& getVertexBuffer() const { return *m_globalVertexBuffer; }
        [[nodiscard]] VulkanBuffer& getIndexBuffer() const { return *m_globalIndexBuffer; }

        [[nodiscard]] bool isUploaded() const { return m_globalVertexBuffer && m_globalIndexBuffer; }
    private:
        void ensureVertexCapacity(uint32_t additionalVertices);
        void ensureIndexCapacity(uint32_t additionalIndices);

        static void growBuffer(std::unique_ptr<VulkanBuffer>& buffer, VkDeviceSize oldSize, VkDeviceSize newSize, VkBufferUsageFlags usage);

        static void uploadToBufferOffset(const void* data, VkDeviceSize size, VkDeviceSize dstOffset, VulkanBuffer& dstBuffer);
    private:
        friend class Mesh;
        std::unique_ptr<VulkanBuffer> m_globalVertexBuffer;
        std::unique_ptr<VulkanBuffer> m_globalIndexBuffer;

        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount = 0;

        uint32_t m_vertexCapacity = 0;
        uint32_t m_indexCapacity = 0;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHREGISTRY_H