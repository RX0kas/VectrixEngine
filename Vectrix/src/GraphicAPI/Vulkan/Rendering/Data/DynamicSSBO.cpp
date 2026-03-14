#include "DynamicSSBO.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
    DynamicSSBO::DynamicSSBO(Device& device, ShaderUniformLayout& layout, uint32_t initialCapacity) : m_device(device), m_layout(layout), m_capacity(initialCapacity) {
        m_framesInFlight = SwapChain::MAX_FRAMES_IN_FLIGHT;

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(m_device.physicalDevice(), &props);
        VkDeviceSize minAlign = props.limits.minStorageBufferOffsetAlignment;

        uint32_t structSize = m_layout.getStructSize();
        m_elementStride = static_cast<uint32_t>((structSize + minAlign - 1) / minAlign * minAlign);

        createDescriptorSetLayout();

        allocateGPUBuffer();

        m_storage.resize(static_cast<size_t>(m_elementStride) * m_capacity * m_framesInFlight, 0);

        m_descriptorSets.resize(m_framesInFlight);
        std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo alloc{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        alloc.descriptorPool     = m_device.descriptorPool();
        alloc.descriptorSetCount = m_framesInFlight;
        alloc.pSetLayouts        = layouts.data();
        vkAllocateDescriptorSets(m_device.device(), &alloc, m_descriptorSets.data());

        updateDescriptorSets();
    }

    DynamicSSBO::~DynamicSSBO() {
        if (m_mapped) vmaUnmapMemory(VulkanContext::instance().getAllocator(), m_allocation);
        m_device.destroyBuffer(m_buffer, m_allocation);
        vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout, nullptr);
    }

    void DynamicSSBO::write(uint32_t frameIndex, uint32_t elementIndex, const void* src) {
        while (elementIndex >= m_capacity) {
            grow();
        }

        size_t offset = (static_cast<size_t>(frameIndex) * m_capacity + elementIndex) * m_elementStride;

        std::memcpy(m_storage.data() + offset, src, m_layout.getStructSize());
    }

    void DynamicSSBO::flush(uint32_t frameIndex) const {
        size_t frameSize = static_cast<size_t>(m_elementStride) * m_capacity;
        size_t offset    = static_cast<size_t>(frameIndex) * frameSize;

        uint8_t* dst = static_cast<uint8_t*>(m_mapped) + offset;
        std::memcpy(dst, m_storage.data() + offset, frameSize);
    }

    void DynamicSSBO::reset(uint32_t frameIndex) {
        size_t frameSize = static_cast<size_t>(m_elementStride) * m_capacity;
        size_t offset    = static_cast<size_t>(frameIndex) * frameSize;
        std::memset(m_storage.data() + offset, 0, frameSize);
    }

    void DynamicSSBO::grow() {
        m_capacity *= 2;

        if (m_mapped) vmaUnmapMemory(VulkanContext::instance().getAllocator(), m_allocation);
        m_device.destroyBuffer(m_buffer, m_allocation);
        m_mapped = nullptr;

        m_storage.resize(static_cast<size_t>(m_elementStride) * m_capacity * m_framesInFlight, 0);

        allocateGPUBuffer();

        updateDescriptorSets();
    }

    void DynamicSSBO::allocateGPUBuffer() {
        VkDeviceSize bufferSize = static_cast<VkDeviceSize>(m_elementStride) * m_capacity * m_framesInFlight;

        m_device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_buffer,
            m_allocation
        );

        vmaMapMemory(VulkanContext::instance().getAllocator(), m_allocation, &m_mapped);
        memset(m_mapped, 0, bufferSize);
    }

    void DynamicSSBO::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding b{};
        b.binding         = 0;
        b.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        b.descriptorCount = 1;
        b.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo li{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        li.bindingCount = 1;
        li.pBindings    = &b;
        vkCreateDescriptorSetLayout(m_device.device(), &li, nullptr, &m_descriptorSetLayout);
    }

    void DynamicSSBO::updateDescriptorSets() const {
        for (uint32_t frame = 0; frame < m_framesInFlight; ++frame) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_buffer;
            bufferInfo.offset = static_cast<VkDeviceSize>(frame) * m_elementStride * m_capacity;
            bufferInfo.range  = static_cast<VkDeviceSize>(m_elementStride) * m_capacity;

            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet          = m_descriptorSets[frame];
            write.dstBinding      = 0;
            write.descriptorCount = 1;
            write.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo     = &bufferInfo;

            vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
        }
    }
} // Vectrix