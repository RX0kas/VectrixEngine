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

        for (auto& tex : m_textures) {
            auto defaultTex = TextureManager::getNotFoundTexture();
            tex = std::dynamic_pointer_cast<VulkanTexture>(defaultTex);
        }

        for (uint32_t frame = 0; frame < m_framesInFlight; ++frame) {
            // SSBO/Uniforms
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_buffer;
            bufferInfo.offset = frame * m_elementStride;
            bufferInfo.range = m_layout.getStructSize();

            VkWriteDescriptorSet writeBuffer{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeBuffer.dstSet = m_descriptorSets[frame];
            writeBuffer.dstBinding = 0;
            writeBuffer.descriptorCount = 1;
            writeBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeBuffer.pBufferInfo = &bufferInfo;

            // Textures
            std::vector<VkDescriptorImageInfo> imageInfos(Texture::getMaxTexture());
            for (uint32_t i = 0; i < Texture::getMaxTexture(); ++i) {
                imageInfos[i] = m_textures[i]->getDescriptorInfo();
            }

            VkWriteDescriptorSet writeTexture{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeTexture.dstSet = m_descriptorSets[frame];
            writeTexture.dstBinding = 1;
            writeTexture.dstArrayElement = 0;
            writeTexture.descriptorCount = Texture::getMaxTexture();
            writeTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeTexture.pImageInfo = imageInfos.data();

            std::array<VkWriteDescriptorSet, 2> writes = { writeBuffer, writeTexture };
            vkUpdateDescriptorSets(m_device.device(), writes.size(), writes.data(), 0, nullptr);
        }
    }

    SSBO::~SSBO()
    {
        if (m_mapped) vmaUnmapMemory(VulkanContext::instance().getAllocator(), m_allocation);
        m_device.destroyBuffer(m_buffer, m_allocation);
        vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout, nullptr);
    }

    void SSBO::createDescriptorSetLayout() {
        std::array<VkDescriptorSetLayoutBinding,2> b = {};

        // SSBO/Uniforms
        b[0].binding = 0;
        b[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        b[0].descriptorCount = 1;
        b[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        b[0].pImmutableSamplers = nullptr;

        // Textures
        b[1].binding = 1;
        b[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        b[1].descriptorCount = Texture::getMaxTexture();
        b[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        b[1].pImmutableSamplers = nullptr;

        std::array<VkDescriptorBindingFlagsEXT, 2> bindingFlags{};
        bindingFlags[0] = 0;
        bindingFlags[1] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
        extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        extendedInfo.bindingCount = b.size();
        extendedInfo.pBindingFlags = bindingFlags.data();

        VkDescriptorSetLayoutCreateInfo li{};
        li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        li.pNext = &extendedInfo;
        li.bindingCount = b.size();
        li.pBindings = b.data();
        li.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        if (vkCreateDescriptorSetLayout(m_device.device(), &li, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            VC_CORE_CRITICAL("Failed to create descriptor set layout");
        }
    }
} // Vectrix
