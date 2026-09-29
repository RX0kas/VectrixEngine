#include "MeshRegistry.h"

#include <algorithm>
#include <iterator>

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

        MeshHandle handle{};
        handle.vertexCount = static_cast<uint32_t>(vertices.size());
        handle.indexCount = static_cast<uint32_t>(indices.size());
        handle.firstVertex = allocateVertices(handle.vertexCount);
        handle.firstIndex = allocateIndices(handle.indexCount);

        VkDeviceSize vertexOffset = static_cast<VkDeviceSize>(handle.firstVertex) * sizeof(Vertex);

        VkDeviceSize indexOffset = static_cast<VkDeviceSize>(handle.firstIndex) * sizeof(uint32_t);

        VkDeviceSize vertexDataSize = vertices.size() * sizeof(Vertex);

        VkDeviceSize indexDataSize = indices.size() * sizeof(uint32_t);

        uploadToBufferOffset(vertices.data(),vertexDataSize,vertexOffset,*m_globalVertexBuffer);

        uploadToBufferOffset(indices.data(),indexDataSize,indexOffset,*m_globalIndexBuffer);

        return handle;
    }

    void MeshRegistry::releaseMesh(const MeshHandle& handle) {
        // Frames in flight may still draw the mesh: a mesh uploaded into its space before they're done would be
        // drawn in its place. The space is given back when the renderer drops this, after those frames
        class PendingRelease {
        public:
            PendingRelease(MeshRegistry& registry, const MeshHandle& handle) : m_registry(registry), m_handle(handle) {}
            PendingRelease(const PendingRelease&) = delete;
            PendingRelease& operator=(const PendingRelease&) = delete;
            ~PendingRelease() { m_registry.freeMeshRanges(m_handle); }
        private:
            MeshRegistry& m_registry;
            MeshHandle m_handle;
        };
        VulkanContext::instance().getRenderer().releaseAfterFrame(std::make_shared<PendingRelease>(*this, handle));
    }

    void MeshRegistry::freeMeshRanges(const MeshHandle& handle) {
        giveBackRange(m_freeVertices, m_vertexCount, {handle.firstVertex, handle.vertexCount});
        giveBackRange(m_freeIndices, m_indexCount, {handle.firstIndex, handle.indexCount});
    }

    uint32_t MeshRegistry::allocateVertices(const uint32_t count) {
        if (const auto first = takeFreeRange(m_freeVertices, count))
            return *first;
        ensureVertexCapacity(count);
        const uint32_t first = m_vertexCount;
        m_vertexCount += count;
        return first;
    }

    uint32_t MeshRegistry::allocateIndices(const uint32_t count) {
        if (const auto first = takeFreeRange(m_freeIndices, count))
            return *first;
        ensureIndexCapacity(count);
        const uint32_t first = m_indexCount;
        m_indexCount += count;
        return first;
    }

    std::optional<uint32_t> MeshRegistry::takeFreeRange(std::vector<Range>& freeRanges, const uint32_t count) {
        for (auto it = freeRanges.begin(); it != freeRanges.end(); ++it) {
            if (it->count < count)
                continue;
            const uint32_t first = it->first;
            it->first += count;
            it->count -= count;
            if (it->count == 0)
                freeRanges.erase(it);
            return first;
        }
        return std::nullopt;
    }

    void MeshRegistry::giveBackRange(std::vector<Range>& freeRanges, uint32_t& used, const Range range) {
        if (range.count == 0)
            return;

        auto it = freeRanges.insert(std::ranges::lower_bound(freeRanges, range.first, {}, &Range::first), range);
        if (const auto next = std::next(it); next != freeRanges.end() && it->first + it->count == next->first) {
            it->count += next->count;
            freeRanges.erase(next);
        }
        if (it != freeRanges.begin()) {
            if (const auto previous = std::prev(it); previous->first + previous->count == it->first) {
                previous->count += it->count;
                freeRanges.erase(it);
            }
        }

        // Free space ending where the used space does is just unused: appending starts there again
        if (!freeRanges.empty() && freeRanges.back().first + freeRanges.back().count == used) {
            used = freeRanges.back().first;
            freeRanges.pop_back();
        }
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

        auto newBuffer = std::make_unique<VulkanBuffer>(newSize,1,
            usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (buffer && oldSize > 0)
            VulkanContext::instance().getDevice().copyBuffer(buffer->getBuffer(),newBuffer->getBuffer(),oldSize);

        // Frames in flight, and the one being recorded, may still draw from the old buffer (a model loaded from an
        // ImGui panel runs after the scene was drawn): destroying it now would invalidate their command buffers
        if (buffer)
            VulkanContext::instance().getRenderer().releaseAfterFrame(std::shared_ptr<VulkanBuffer>(std::move(buffer)));
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
