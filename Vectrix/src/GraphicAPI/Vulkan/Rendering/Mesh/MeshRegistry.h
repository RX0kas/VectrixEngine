#ifndef VECTRIXWORKSPACE_MESHREGISTRY_H
#define VECTRIXWORKSPACE_MESHREGISTRY_H
#include <memory>
#include <optional>
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

        /**
         * @brief Give a mesh's space in the shared buffers back, for the meshes uploaded next
         * @note The space is reused only once the GPU is done with the frames that may still draw the mesh
         */
        void releaseMesh(const MeshHandle& handle);

        [[nodiscard]] VulkanBuffer& getVertexBuffer() const { return *m_globalVertexBuffer; }
        [[nodiscard]] VulkanBuffer& getIndexBuffer() const { return *m_globalIndexBuffer; }

        [[nodiscard]] bool isUploaded() const { return m_globalVertexBuffer && m_globalIndexBuffer; }
    private:
        /// Consecutive vertices or indices of a shared buffer
        struct Range {
            uint32_t first;
            uint32_t count;
        };

        /// Takes count elements from a released range (first fit), or from the end of the used space
        uint32_t allocateVertices(uint32_t count);
        uint32_t allocateIndices(uint32_t count);
        static std::optional<uint32_t> takeFreeRange(std::vector<Range>& freeRanges, uint32_t count);
        /// Adds range to the sorted free list, merging it with its neighbours; free space at the end shrinks used
        static void giveBackRange(std::vector<Range>& freeRanges, uint32_t& used, Range range);
        /// Makes the mesh's ranges reusable right away (see releaseMesh for the deferred version)
        void freeMeshRanges(const MeshHandle& handle);

        void ensureVertexCapacity(uint32_t additionalVertices);
        void ensureIndexCapacity(uint32_t additionalIndices);

        static void growBuffer(std::unique_ptr<VulkanBuffer>& buffer, VkDeviceSize oldSize, VkDeviceSize newSize, VkBufferUsageFlags usage);

        static void uploadToBufferOffset(const void* data, VkDeviceSize size, VkDeviceSize dstOffset, VulkanBuffer& dstBuffer);
    private:
        friend class Mesh;
        std::unique_ptr<VulkanBuffer> m_globalVertexBuffer;
        std::unique_ptr<VulkanBuffer> m_globalIndexBuffer;

        /// End of the used space: everything past it is free, released ranges before it are in the free lists
        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount = 0;
        std::vector<Range> m_freeVertices; ///< Sorted by first, never adjacent to each other or to the end
        std::vector<Range> m_freeIndices;

        uint32_t m_vertexCapacity = 0;
        uint32_t m_indexCapacity = 0;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_MESHREGISTRY_H