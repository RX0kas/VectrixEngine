#include "SSBO.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"


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


        // creation descriptor set
        createDescriptorSetLayout();

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
            bi.offset = i * m_elementStride;
            bi.range = m_layout.getStructSize();

            VkWriteDescriptorSet w[1] = {};
            w[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            w[0].dstSet = m_descriptorSets[i];
            w[0].dstBinding = 0;
            w[0].descriptorCount = 1;
            w[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            w[0].pBufferInfo = &bi;

            vkUpdateDescriptorSets(m_device.device(), 1, w, 0, nullptr);
        }
    }

    SSBO::~SSBO()
    {
        if (m_mapped) vmaUnmapMemory(VulkanContext::instance().getAllocator(), m_allocation);
        m_device.destroyBuffer(m_buffer, m_allocation);
        vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout, nullptr);
    }

    void SSBO::setTexture(const VulkanTexture *texture) const {
        VulkanRenderer& r = VulkanContext::instance().getRenderer();
        vkWaitForFences(m_device.device(), 1, &r.getInFlightFences()[r.getFrameIndex()], VK_TRUE, UINT64_MAX);

        VkDescriptorImageInfo imageInfo = texture->getDescriptorInfo();
        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = m_descriptorSets[r.getFrameIndex()];
        write.dstBinding = 1;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
    }

    void SSBO::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding b[2] = {};

        b[0].binding = 0;
        b[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        b[0].descriptorCount = 1;
        b[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        b[1].binding = 1;
        b[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        b[1].descriptorCount = 1;
        b[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        li.bindingCount = 2;
        li.pBindings = b;

        if (vkCreateDescriptorSetLayout(m_device.device(), &li, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            VC_CORE_CRITICAL("Failed to create descriptor set layout");
        }
    }
} // Vectrix
