#include "vcpch.h"

#include "VulkanBuffer.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"

/*
 * Thanks to Sascha Willems
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

namespace Vectrix {
    // VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1
    VulkanVertexBuffer::VulkanVertexBuffer(const std::vector<Vertex>& vertices, uint32_t size)
    {
        //VC_CORE_ASSERT(sizeof(vert) >= 3, "Vertex count must be at least 3");
        //VC_CORE_ASSERT(sizeof(vert)%3==0, "Vertex count must be a multiple of 3");


        VC_CORE_TRACE("vertex: {0}", vertices);

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
        VulkanContext::instance()._VertexBuffer.push_back(this);
    }

    void VulkanVertexBuffer::draw() {
        vkCmdDraw(VulkanContext::instance().getRenderer().getCurrentCommandBuffer(), _vertexCount, 1, 0, 0);
    }

    void VulkanVertexBuffer::bind() {
        VkBuffer buffers[] = {buffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(VulkanContext::instance().getRenderer().getCurrentCommandBuffer(), 0, 1, buffers, offsets);
        _enable = true;
    }

    void VulkanIndexBuffer::draw() {
        VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
        VC_CORE_ASSERT(cmd != VK_NULL_HANDLE,"CommandBuffer invalide");
        vkCmdDrawIndexed(cmd, _IndexCount, 1, 0, 0, 0);
    }

    void VulkanIndexBuffer::bind() {
        VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
        VC_CORE_ASSERT(cmd != VK_NULL_HANDLE,"CommandBuffer invalide");
        vkCmdBindIndexBuffer(cmd, buffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        _enable = true;
    }

    VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count) {
        _IndexCount = static_cast<uint32_t>(sizeof(indices));

        VC_CORE_ASSERT(_IndexCount > 0,"Indices array is empty");

        VkDeviceSize bufferSize = sizeof(indices[0]) * _IndexCount;
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
            indexSize,
            _IndexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(indices);

        buffer = std::make_unique<Buffer>(
            indexSize,
            _IndexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VulkanContext::instance().getDevice().copyBuffer(stagingBuffer.getBuffer(), buffer->getBuffer(), bufferSize);
        VulkanContext::instance()._IndexBuffer.push_back(this);
    }


    VulkanIndexBuffer::~VulkanIndexBuffer() = default;

    VulkanVertexBuffer::~VulkanVertexBuffer() = default;


    ////////////////////////////////////
    //             Vertex             //
    ////////////////////////////////////

    /*std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }


	// Returns the attribute descriptions for the vertex input
	// So the layout of the vertex shader
    std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2); // Position and Color
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VULKAN_FORMAT_VEC2;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VULKAN_FORMAT_VEC2;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }*/

    // Describes at which rate to load data from memory throughout the vertices. 
    // It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
    std::vector<VkVertexInputBindingDescription> VulkanVertexBuffer::getBindingDescriptions(const BufferLayout& layout) {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = layout.getStride();
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }


    // Returns the attribute descriptions for the vertex input
    // So the layout of the vertex shader
    std::vector<VkVertexInputAttributeDescription> VulkanVertexBuffer::getAttributeDescriptions(const BufferLayout& layout) {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(layout.getElements().size());
        VC_CORE_TRACE("getAttributeDescriptions size: {0}", layout.getElements().size());
        for (size_t i = 0; i < layout.getElements().size(); i++) {
            const auto& element = layout.getElements()[i];
            attributeDescriptions[i].binding = 0;
            attributeDescriptions[i].location = static_cast<uint32_t>(i);
            attributeDescriptions[i].format = shaderDataTypeToVkFormat(element.Type);
            attributeDescriptions[i].offset = element.Offset;
		}
        VC_CORE_TRACE(attributeDescriptions.data()->format);
		return attributeDescriptions;
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

    Buffer::Buffer(
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : device{ VulkanContext::instance().getDevice()},
        instanceSize{ instanceSize },
        instanceCount{ instanceCount },
        usageFlags{ usageFlags },
        memoryPropertyFlags{ memoryPropertyFlags } {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    Buffer::~Buffer() {
        unmap();
        vkDeviceWaitIdle(device.device()); // TODO: Find if there is a better method the wait until the complete execution of the buffer
        vkDestroyBuffer(device.device(), buffer, nullptr);
        vkFreeMemory(device.device(), memory, nullptr);
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
        VC_CORE_ASSERT(buffer && memory, "Called map on buffer before create");
        return vkMapMemory(device.device(), memory, offset, size, 0, &mapped);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void Buffer::unmap() {
        if (mapped) {
            vkUnmapMemory(device.device(), memory);
            mapped = nullptr;
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
        VC_CORE_ASSERT(mapped, "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        }
        else {
            char* memOffset = (char*)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
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
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(device.device(), 1, &mappedRange);
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
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(device.device(), 1, &mappedRange);
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
            buffer,
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
    VkResult Buffer::flushIndex(int index) { return flush(alignmentSize, index * alignmentSize); }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignmentSize, index * alignmentSize);
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
        return invalidate(alignmentSize, index * alignmentSize);
    }
}