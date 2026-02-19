#include "vcpch.h"

#include "GraphicAPI/Vulkan/Rendering/VulkanBuffer.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

/*
 * Thanks to Sascha Willems
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

namespace Vectrix {

    // Vertex && Index Buffer
    VulkanVertexBuffer::VulkanVertexBuffer(const std::vector<Vertex>& vertices, uint32_t size)
    {
        _vertexCount = static_cast<uint32_t>(vertices.size());
        VkDeviceSize bufferSize = sizeof(vertices[0]) * _vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        Buffer stagingBuffer{
            vertexSize,
            _vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*) vertices.data());

        buffer = std::make_unique<Buffer>(vertexSize, _vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VulkanContext::instance().getDevice().copyBuffer(stagingBuffer.getBuffer(), buffer->getBuffer(), bufferSize);
    }

    void VulkanVertexBuffer::draw() {
        vkCmdDraw(VulkanContext::instance().getRenderer().getCurrentCommandBuffer(), _vertexCount, 1, 0, 0);
    }

    void VulkanVertexBuffer::bind() {
        VC_CORE_ASSERT(m_Layout.getStride() == sizeof(Vertex), "Vertex stride mismatch: layout != sizeof(Vertex): layout = {}, sizeof(Vertex)={}",m_Layout.getStride(),sizeof(Vertex));
        VkBuffer buffers[] = {buffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(VulkanContext::instance().getRenderer().getCurrentCommandBuffer(), 0, 1, buffers, offsets);
        _enable = true;
    }

    void VulkanIndexBuffer::draw() {
        VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
        VC_CORE_ASSERT(cmd != VK_NULL_HANDLE, "CommandBuffer invalide");
        VC_CORE_ASSERT(buffer && buffer->getBuffer() != VK_NULL_HANDLE, "Index buffer invalide");
        VC_CORE_ASSERT(_IndexCount > 0, "IndexCount invalide");

        vkCmdDrawIndexed(cmd, _IndexCount, 1, 0, 0, 0);
    }


    void VulkanIndexBuffer::bind() {
        VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
        VC_CORE_ASSERT(cmd != VK_NULL_HANDLE, "CommandBuffer invalide");
        VC_CORE_ASSERT(buffer && buffer->getBuffer() != VK_NULL_HANDLE, "Index buffer VkBuffer invalide");
        VC_CORE_ASSERT(_IndexCount > 0, "IndexCount invalide");


        vkCmdBindIndexBuffer(cmd, buffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        _enable = true;
    }


    VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, uint32_t count) {
        VC_CORE_ASSERT(indices != nullptr, "indices pointer is null");
        VC_CORE_ASSERT(count > 0, "Indices array is empty");

        _IndexCount = count;
        VkDeviceSize bufferSize = sizeof(indices[0]) * _IndexCount;

        // Utiliser Buffer comme pour VertexBuffer
        Buffer stagingBuffer{
            sizeof(indices[0]),
            _IndexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices);
        stagingBuffer.unmap(); // Important: unmapper après écriture

        buffer = std::make_unique<Buffer>(
            sizeof(indices[0]),
            _IndexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VulkanContext::instance().getDevice().copyBuffer(stagingBuffer.getBuffer(),buffer->getBuffer(),bufferSize);
    }



    VulkanIndexBuffer::~VulkanIndexBuffer() = default;

    VulkanVertexBuffer::~VulkanVertexBuffer() = default;


    ////////////////////////////////////
    //             Vertex             //
    ////////////////////////////////////
    // Describes at which rate to load data from memory throughout the vertices. 
    // It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
    std::vector<VkVertexInputBindingDescription> VulkanVertexBuffer::getBindingDescriptions(const BufferLayout& layout) {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = layout.getStride(); // DOIT être sizeof(Vertex)
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return { binding };
    }



    // Returns the attribute descriptions for the vertex input
    // So the layout of the vertex shader
    std::vector<VkVertexInputAttributeDescription> VulkanVertexBuffer::getAttributeDescriptions(const BufferLayout& layout) {
        std::vector<VkVertexInputAttributeDescription> attributes;

        uint32_t location = 0;
        for (const auto& element : layout.getElements()) {
            VkVertexInputAttributeDescription attr{};
            attr.binding = 0;
            attr.location = location++;
            attr.offset = element.Offset;

            switch (element.Type) {
                case ShaderDataType::Float:
                    attr.format = VK_FORMAT_R32_SFLOAT;
                    break;
                case ShaderDataType::Float2:
                    attr.format = VK_FORMAT_R32G32_SFLOAT;
                    break;
                case ShaderDataType::Float3:
                    attr.format = VK_FORMAT_R32G32B32_SFLOAT;
                    break;
                case ShaderDataType::Float4:
                    attr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    break;
            }

            attributes.push_back(attr);
        }

        return attributes;
    }


    ////////////////////////////////////
    //          Buffer Class          //
    ////////////////////////////////////
    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instanceSize The size of an instance
     * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */
    VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    Buffer::Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
            : m_device{ VulkanContext::instance().getDevice()}, m_instanceSize{ instanceSize }, m_instanceCount{ instanceCount }, m_usageFlags{ usageFlags }, m_memoryPropertyFlags{ memoryPropertyFlags } {
        m_alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        m_bufferSize = m_alignmentSize * instanceCount;

        m_device.createBuffer(m_bufferSize, usageFlags, memoryPropertyFlags, m_buffer, m_allocation);
    }

    Buffer::~Buffer() {
        unmap();
        if (m_buffer != VK_NULL_HANDLE) {
            m_device.destroyBuffer(m_buffer, m_allocation);
        }
    }

    /**
     * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
     * buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the buffer mapping call
     */
    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
        VC_CORE_ASSERT(m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE, "Called map on buffer before create");
        return vmaMapMemory(m_device.getAllocator(), m_allocation, &m_mapped);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void Buffer::unmap() {
        if (m_mapped) {
            vmaUnmapMemory(m_device.getAllocator(), m_allocation);
            m_mapped = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */
    void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
        VC_CORE_ASSERT(m_mapped, "Cannot copy to unmapped buffer");

        char* mem = static_cast<char*>(m_mapped) + offset;

        if (size == VK_WHOLE_SIZE) {
            memcpy(m_mapped, data, m_bufferSize);
        } else {
            memcpy(mem, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
     * complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */
    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

        VmaAllocationInfo pAllocationInfo;
        vmaGetAllocationInfo(m_device.getAllocator(), m_allocation,&pAllocationInfo);

        mappedRange.memory = pAllocationInfo.deviceMemory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(m_device.device(), 1, &mappedRange);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */
    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

        VmaAllocationInfo pAllocationInfo;
        vmaGetAllocationInfo(m_device.getAllocator(), m_allocation,&pAllocationInfo);

        mappedRange.memory = pAllocationInfo.deviceMemory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(m_device.device(), 1, &mappedRange);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{
            m_buffer,
            offset,
            size,
        };
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    VkResult Buffer::flushIndex(int index) { return flush(m_alignmentSize, index * m_alignmentSize); }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(m_alignmentSize, index * m_alignmentSize);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */
    VkResult Buffer::invalidateIndex(int index) {
        return invalidate(m_alignmentSize, index * m_alignmentSize);
    }
}