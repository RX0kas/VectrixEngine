#include "SSBO.h"


namespace Vectrix {
    SSBO::SSBO(Device& device,const ShaderUniformLayout& layout, VkDescriptorSetLayout providedLayout) : m_device(device),m_layout(layout) {
        // CPU Buffer
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device.physicalDevice(), &props);
        m_framesInFlight = SwapChain::MAX_FRAMES_IN_FLIGHT;
        m_elementStride = props.limits.minStorageBufferOffsetAlignment;
        createBuffer();
        m_bufferSize = static_cast<VkDeviceSize>(m_elementStride) * m_framesInFlight;


        // GPU Buffer
        // create buffer and memory
        m_device.createBuffer(m_bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_buffer,
            m_memory);

        vkMapMemory(m_device.device(), m_memory, 0, m_bufferSize, 0, &m_mapped);

        // create descriptor set layout if not provided
        if (providedLayout == VK_NULL_HANDLE) {
            VkDescriptorSetLayoutBinding b{};
            b.binding = 0;
            b.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            b.descriptorCount = 1;
            b.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            b.pImmutableSamplers = nullptr;
            VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            li.bindingCount = 1;
            li.pBindings = &b;
            vkCreateDescriptorSetLayout(m_device.device(), &li, nullptr, &m_descriptorSetLayout);
            m_layoutWasCreatedHere = true;
        } else {
            m_descriptorSetLayout = providedLayout;
            m_layoutWasCreatedHere = false;
        }

        // allocate descriptor sets
        m_descriptorSets.resize(m_framesInFlight);
        std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc.descriptorPool = m_device.descriptorPool();
        alloc.descriptorSetCount = m_framesInFlight;
        alloc.pSetLayouts = layouts.data();
        if (vkAllocateDescriptorSets(m_device.device(), &alloc, m_descriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate SSBO descriptor sets");

        // write descriptor sets
        for (uint32_t i = 0; i < m_framesInFlight; ++i) {
            VkDescriptorBufferInfo bi{};
            bi.buffer = m_buffer;
            bi.offset = (VkDeviceSize)i * m_elementStride;
            bi.range = m_elementStride;

            VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            w.dstSet = m_descriptorSets[i];
            w.dstBinding = 0;
            w.descriptorCount = 1;
            w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            w.pBufferInfo = &bi;
            vkUpdateDescriptorSets(m_device.device(), 1, &w, 0, nullptr);
        }
    }


    SSBO::~SSBO()
    {
        if (m_mapped) vkUnmapMemory(m_device.device(), m_memory);
        if (m_buffer) vkDestroyBuffer(m_device.device(), m_buffer, nullptr);
        if (m_memory) vkFreeMemory(m_device.device(), m_memory, nullptr);
        if (m_layoutOwner()) vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout, nullptr);
    }

    void SSBO::createBuffer() {
        // compute GPU alignment (device limit)
        VkDeviceSize alignment = m_elementStride ? m_elementStride : 1;
        m_elementStride = static_cast<uint32_t>((m_layout.getStructSize()+alignment-1)/alignment * alignment);
        m_storage.resize(static_cast<size_t>(m_elementStride) * m_framesInFlight);
    }
} // Vectrix
