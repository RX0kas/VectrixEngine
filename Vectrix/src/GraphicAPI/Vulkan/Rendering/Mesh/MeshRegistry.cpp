#include "MeshRegistry.h"

#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Mesh/Model.h"

namespace Vectrix {
    MeshRegistry::MeshRegistry() {
        m_uploaded = false;
    }

    MeshRegistry::~MeshRegistry() {
        VC_PROFILER_FUNCTION();
        m_globalIndexBuffer.reset();
        m_globalVertexBuffer.reset();
    }

    MeshHandle MeshRegistry::registerMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT(!m_uploaded, "Cannot register mesh after GPU upload!");

        MeshHandle handle {
            .firstIndex   = static_cast<uint32_t>(m_pendingIndices.size()),
            .indexCount   = static_cast<uint32_t>(indices.size()),
            .vertexOffset = static_cast<int32_t>(m_pendingVertices.size())
        };

        m_pendingVertices.insert(m_pendingVertices.end(), vertices.begin(), vertices.end());
        m_pendingIndices.insert(m_pendingIndices.end(),   indices.begin(),  indices.end());

        return handle;
    }

    void MeshRegistry::uploadToGPU() {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT(!m_uploaded, "Already uploaded!");
        if (m_pendingVertices.empty() || m_pendingIndices.empty()) {return;}

        const VkDeviceSize vertexDataSize = m_pendingVertices.size() * sizeof(Vertex);
        const VkDeviceSize indexDataSize  = m_pendingIndices.size()  * sizeof(uint32_t);


        uploadBuffer(m_pendingVertices.data(), vertexDataSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,m_globalVertexBuffer);

        uploadBuffer(m_pendingIndices.data(), indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,m_globalIndexBuffer);

        m_pendingVertices.clear();
        m_pendingVertices.shrink_to_fit();
        m_pendingIndices.clear();
        m_pendingIndices.shrink_to_fit();

        m_uploaded = true;
    }

    void MeshRegistry::uploadBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, std::unique_ptr<VulkanBuffer>& outBuffer) {
        VC_PROFILER_FUNCTION();
        VulkanBuffer stagingBuffer(size, 1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        stagingBuffer.map();
        stagingBuffer.writeToBuffer(data, size);

        outBuffer = std::make_unique<VulkanBuffer>(size, 1,
            usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        VulkanContext::instance().getDevice().copyBuffer(stagingBuffer.getBuffer(), outBuffer->getBuffer(), size);
    }
}
