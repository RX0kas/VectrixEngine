#include "SSBO.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"


namespace Vectrix {
    SSBO::SSBO(Device& device, ShaderUniformLayout& layout) : m_device(device), m_layout(layout) {
        m_framesInFlight = SwapChain::MAX_FRAMES_IN_FLIGHT;

        // Alignement
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device.physicalDevice(), &props);
        VkDeviceSize minAlignment = props.limits.minStorageBufferOffsetAlignment;

        m_elementStride = static_cast<uint32_t>(
            (m_layout.getStructSize() + minAlignment - 1) / minAlignment * minAlignment
        );

        m_storage.resize(static_cast<size_t>(m_elementStride) * m_framesInFlight);
        m_bufferSize = static_cast<VkDeviceSize>(m_elementStride) * m_framesInFlight;

        // GPU Buffer
        m_device.createBuffer(m_bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_allocation);

        vmaMapMemory(VulkanContext::instance().getAllocator(), m_allocation, &m_mapped);
        memset(m_mapped, 0, m_bufferSize);

        VkDescriptorSetLayoutBinding b{};
        b.binding = 0;
        b.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        b.descriptorCount = 1;

        b.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        b.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        li.bindingCount = 1;
        li.pBindings = &b;

        if (vkCreateDescriptorSetLayout(m_device.device(), &li, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            VC_CORE_CRITICAL("Failed to create descriptor set layout");
        }

        m_descriptorSets.resize(m_framesInFlight);
        std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc.descriptorPool = m_device.descriptorPool();
        alloc.descriptorSetCount = m_framesInFlight;
        alloc.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(m_device.device(), &alloc, m_descriptorSets.data()) != VK_SUCCESS) {
            VC_CORE_CRITICAL("Failed to allocate SSBO descriptor sets");
        }

        for (uint32_t i = 0; i < m_framesInFlight; ++i) {
            VkDescriptorBufferInfo bi{};
            bi.buffer = m_buffer;
            bi.offset = static_cast<VkDeviceSize>(i) * m_elementStride;
            bi.range = m_layout.getStructSize();

            VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            w.dstSet = m_descriptorSets[i];
            w.dstBinding = 0;
            w.dstArrayElement = 0;
            w.descriptorCount = 1;
            w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            w.pBufferInfo = &bi;

            vkUpdateDescriptorSets(m_device.device(), 1, &w, 0, nullptr);
        }
    }

    SSBO::~SSBO()
    {
        if (m_mapped) vmaUnmapMemory(VulkanContext::instance().getAllocator(), m_allocation);
        m_device.destroyBuffer(m_buffer, m_allocation);
        vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout, nullptr);
    }
} // Vectrix
