#include "MeshRegistry.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Mesh/MeshHandle.h"

namespace Vectrix {
    MeshRegistry::MeshRegistry() = default;

    MeshRegistry::~MeshRegistry() {
        m_globalVertexBuffer.reset();
        m_globalIndexBuffer.reset();
    }

    MeshHandle MeshRegistry::uploadMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT(!vertices.empty(), "Cannot upload mesh with no vertices");
        VC_CORE_ASSERT(!indices.empty(), "Cannot upload mesh with no indices");

        ensureVertexCapacity(static_cast<uint32_t>(vertices.size()));
        ensureIndexCapacity(static_cast<uint32_t>(indices.size()));

        MeshHandle handle{};
        handle.firstVertex = m_vertexCount;
        handle.firstIndex = m_indexCount;
        handle.vertexCount = static_cast<uint32_t>(vertices.size());
        handle.indexCount = static_cast<uint32_t>(indices.size());

        VkDeviceSize vertexOffset = static_cast<VkDeviceSize>(handle.firstVertex) * sizeof(Vertex);

        VkDeviceSize indexOffset = static_cast<VkDeviceSize>(handle.firstIndex) * sizeof(uint32_t);

        VkDeviceSize vertexDataSize = vertices.size() * sizeof(Vertex);

        VkDeviceSize indexDataSize = indices.size() * sizeof(uint32_t);

        uploadToBufferOffset(vertices.data(),vertexDataSize,vertexOffset,*m_globalVertexBuffer);

        uploadToBufferOffset(indices.data(),indexDataSize,indexOffset,*m_globalIndexBuffer);

        m_vertexCount += handle.vertexCount;
        m_indexCount += handle.indexCount;

        return handle;
    }

    void MeshRegistry::ensureVertexCapacity(uint32_t additionalVertices) {
        uint32_t required = m_vertexCount + additionalVertices;

        if (required <= m_vertexCapacity)
            return;

        uint32_t newCapacity = m_vertexCapacity == 0 ? 4096 : m_vertexCapacity;

        while (newCapacity < required)
            newCapacity *= 2;

        VkDeviceSize oldSize = static_cast<VkDeviceSize>(m_vertexCapacity) * sizeof(Vertex);

        VkDeviceSize newSize = static_cast<VkDeviceSize>(newCapacity) * sizeof(Vertex);

        growBuffer(m_globalVertexBuffer,oldSize,newSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        m_vertexCapacity = newCapacity;
    }

    void MeshRegistry::ensureIndexCapacity(uint32_t additionalIndices) {
        uint32_t required = m_indexCount + additionalIndices;

        if (required <= m_indexCapacity)
            return;

        uint32_t newCapacity = m_indexCapacity == 0 ? 8192 : m_indexCapacity;

        while (newCapacity < required)
            newCapacity *= 2;

        VkDeviceSize oldSize = static_cast<VkDeviceSize>(m_indexCapacity) * sizeof(uint32_t);

        VkDeviceSize newSize = static_cast<VkDeviceSize>(newCapacity) * sizeof(uint32_t);

        growBuffer(m_globalIndexBuffer,oldSize,newSize,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        m_indexCapacity = newCapacity;
    }

    void MeshRegistry::growBuffer(std::unique_ptr<VulkanBuffer>& buffer, VkDeviceSize oldSize, VkDeviceSize newSize, VkBufferUsageFlags usage) {
        VC_PROFILER_FUNCTION();

        // TODO LATER: Place the old buffer in a “deletion queue” on a per-frame basis and do not destroy it until MAX_FRAMES_IN_FLIGHT has elapsed
        vkDeviceWaitIdle(VulkanContext::instance().getDevice().device());

        auto newBuffer = std::make_unique<VulkanBuffer>(newSize,1,
            usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (buffer && oldSize > 0)
            VulkanContext::instance().getDevice().copyBuffer(buffer->getBuffer(),newBuffer->getBuffer(),oldSize);


        buffer = std::move(newBuffer);
    }

    void MeshRegistry::uploadToBufferOffset(const void* data, VkDeviceSize size, VkDeviceSize dstOffset, VulkanBuffer& dstBuffer) {
        VC_PROFILER_FUNCTION();

        VulkanBuffer stagingBuffer(
            size,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(data, size);

        VulkanContext::instance().getDevice().copyBuffer(stagingBuffer.getBuffer(),dstBuffer.getBuffer(),size,0,dstOffset);
    }
}
