#include "vcpch.h"

#include "GraphicAPI/Vulkan/Rendering/Data/VulkanBuffer.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Debug/Profiler.h"

/*
 * Thanks to Sascha Willems
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

namespace Vectrix {

    ////////////////////////////////////
    //             Vertex             //
    ////////////////////////////////////
    // Describes at which rate to load data from memory throughout the vertices. 
    // It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
    std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions(const BufferLayout& layout) {
        VC_PROFILER_FUNCTION();
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = layout.getStride();
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return { binding };
    }



    // Returns the attribute descriptions for the vertex input
    // So the layout of the vertex shader
    std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions(const BufferLayout& layout) {
        VC_PROFILER_FUNCTION();
        std::vector<VkVertexInputAttributeDescription> attributes;

        uint32_t location = 0;
        for (const auto& element : layout.getElements()) {
            VkVertexInputAttributeDescription attr{};
            attr.binding = 0;
            attr.location = location++;
            attr.offset = element.offset;

            switch (element.type) {
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
                case ShaderDataType::None:
                    attr.format = VK_FORMAT_UNDEFINED;
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
    VkDeviceSize VulkanBuffer::getAlignment(const VkDeviceSize instanceSize, const VkDeviceSize minOffsetAlignment) {
        VC_PROFILER_FUNCTION();
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    VulkanBuffer::VulkanBuffer(const VkDeviceSize instanceSize, const uint32_t instanceCount, const VkBufferUsageFlags usageFlags, const VkMemoryPropertyFlags memoryPropertyFlags, const VkDeviceSize minOffsetAlignment)
            : m_device{ VulkanContext::instance().getDevice()} {
        VC_PROFILER_FUNCTION();
        m_bufferSize = getAlignment(instanceSize, minOffsetAlignment) * instanceCount;

        m_device.createBuffer(m_bufferSize, usageFlags, memoryPropertyFlags, m_buffer, m_allocation);
    }

    VulkanBuffer::~VulkanBuffer() {
        VC_PROFILER_FUNCTION();
        unmap();
        if (m_buffer != VK_NULL_HANDLE) {
            m_device.destroyBuffer(m_buffer, m_allocation);
        }
    }

    /**
     * Map the whole buffer. If successful, the mapped pointer covers the complete buffer range.
     *
     * @return VkResult of the buffer mapping call
     */
    VkResult VulkanBuffer::map() {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT(m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE, "Called map on buffer before create");
        return vmaMapMemory(m_device.getBufferAllocator(), m_allocation, &m_mapped);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void VulkanBuffer::unmap() {
        VC_PROFILER_FUNCTION();
        if (m_mapped) {
            vmaUnmapMemory(m_device.getBufferAllocator(), m_allocation);
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
    void VulkanBuffer::writeToBuffer(const void* data, const VkDeviceSize size, const VkDeviceSize offset) const {
        VC_CORE_ASSERT(m_mapped, "Cannot copy to unmapped buffer");

        char* mem = static_cast<char*>(m_mapped) + offset;

        if (size == VK_WHOLE_SIZE) {
            memcpy(m_mapped, data, m_bufferSize);
        } else {
            memcpy(mem, data, size);
        }
    }
}
