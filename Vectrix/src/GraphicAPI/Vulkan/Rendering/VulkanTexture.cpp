#include "VulkanTexture.h"

#include <algorithm>
#include <filesystem>

#include "Core/Device.h"
#include "stb_image.h"
#include "GraphicAPI/Vulkan/ImGui/imgui_impl_vulkan.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Utils/Data.h"

namespace Vectrix {

    uint32_t VulkanTexture::s_numberTexture = 1;

    static VkDescriptorSet createImGuiTextureDescriptor(Device& device, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout) {
        VkDescriptorSetLayout layout = ImGui_ImplVulkan_GetTextureDescriptorSetLayout();

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = device.descriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
            VC_CORE_ERROR("Failed to allocate ImGui texture descriptor set");
            return VK_NULL_HANDLE;
        }

        ImGui_ImplVulkan_WriteTextureDescriptor(descriptorSet, sampler, imageView, imageLayout);
        return descriptorSet;
    }

    static void destroyImGuiTextureDescriptor(Device& device, VkDescriptorSet descriptorSet) {
        if (descriptorSet == VK_NULL_HANDLE) return;
        vkFreeDescriptorSets(device.device(), device.descriptorPool(), 1, &descriptorSet);
    }

    VulkanTexture::VulkanTexture(const std::string &name, const std::string &path) : m_device(VulkanContext::instance().getDevice()),m_name(name) {
        VC_PROFILER_FUNCTION();
        stbi_uc* pixels = stbi_load(path.c_str(), &m_width, &m_height, &m_channel, STBI_rgb_alpha);

        if (!pixels) {
            // A corrupt/unsupported image must not take the app down: show the not_found image instead,
            // like Texture::create already does for a missing file
            VC_CORE_ERROR_NO_EXIT("Failed to load texture image {}: {}, using default Texture instead", path, stbi_failure_reason());
            pixels = stbi_load_from_memory(getNotFoundTextureData(), getNotFoundTextureSize(), &m_width, &m_height, &m_channel, STBI_rgb_alpha);
            VC_CORE_ASSERT(pixels, "Failed to load embedded not_found texture");
        }
        m_imageSize = m_width * m_height * 4;
        createTexture(pixels,STBI_rgb_alpha);

        m_id = s_numberTexture++;
    }

    /**
     * Used to createDefaultTexture
     */
    VulkanTexture::VulkanTexture() : m_device(VulkanContext::instance().getDevice()),m_name("DefaultTexture") {
        VC_PROFILER_FUNCTION();
        VC_CORE_INFO("Creating not_found texture");
        int x, y, channels;
        stbi_uc* pixels = stbi_load_from_memory(getNotFoundTextureData(), getNotFoundTextureSize(), &x, &y, &channels, 4);
        if (!pixels) {
            VC_CORE_ERROR("Failed to load embedded not_found texture");
        }
        m_width = x;
        m_height = y;
        m_channel = 4;
        m_imageSize = m_width * m_height * 4;
        createTexture(pixels,4);
        m_id = 0;
    }

    void VulkanTexture::createTexture(stbi_uc *pixels, int channels) {
        VC_PROFILER_FUNCTION();
        VkFormat f = VK_FORMAT_UNDEFINED;
        switch (channels) {
            case 1:
                f = VK_FORMAT_R8_SRGB; break;
            case 2:
                f = VK_FORMAT_R8G8_SRGB; break;
            case 3:
                f = VK_FORMAT_R8G8B8_SRGB; break;
            case 4:
                f = VK_FORMAT_R8G8B8A8_SRGB; break;
            default:
                VC_CORE_WARN("Unsupported channels number");
        }

        m_format = f;
        // staging buffer
        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;
        m_device.createBuffer(
            m_imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingAllocation
        );
        // copy to staging buffer. It was allocated by the buffer allocator (createBuffer's default): mapping
        // it through another VmaAllocator is undefined behaviour
        void* data;
        vmaMapMemory(m_device.getBufferAllocator(), stagingAllocation, &data);
        memcpy(data, pixels, m_imageSize);
        vmaUnmapMemory(m_device.getBufferAllocator(), stagingAllocation);
        stbi_image_free(pixels);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(m_width);
        imageInfo.extent.height = static_cast<uint32_t>(m_height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = f;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Texels are laid out in an implementation defined order for optimal access
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Not usable by the GPU and the very first transition will discard the texels
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        // No dedicated memory per texture: VMA already gives large images their own block, and one
        // VkDeviceMemory per texture runs into maxMemoryAllocationCount (often 4096)
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        if (vmaCreateImage(VulkanContext::instance().getTextureAllocator(),&imageInfo,&allocationCreateInfo,&m_image,&m_allocation,nullptr) != VK_SUCCESS) {
            VC_CORE_CRITICAL("Failed to create texture image");
        }

        uploadPixels(stagingBuffer);

        m_device.destroyBuffer(stagingBuffer, stagingAllocation);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = f;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
            VC_CORE_ERROR("Failed to create texture image view");
        }

        const VulkanSettings::Textures& texSettings = VulkanContext::instance().settings().textures;
        const bool useAnisotropy = texSettings.anisotropy > 0.0f;

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = texSettings.filter;
        samplerInfo.minFilter = texSettings.filter;
        samplerInfo.addressModeU = texSettings.wrapMode;
        samplerInfo.addressModeV = texSettings.wrapMode;
        samplerInfo.addressModeW = texSettings.wrapMode;
        samplerInfo.anisotropyEnable = useAnisotropy ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = useAnisotropy
            ? std::min(texSettings.anisotropy, m_device.properties.limits.maxSamplerAnisotropy)
            : 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
            VC_CORE_ERROR("Failed to create texture sampler");
        }
    }

    void VulkanTexture::uploadPixels(VkBuffer stagingBuffer) {
        VC_PROFILER_FUNCTION();
        // The two layout transitions and the copy share one command buffer, so a texture load waits
        // on the GPU once instead of three times
        VkCommandBuffer cmd = m_device.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region{};
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.imageExtent = { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1 };
        vkCmdCopyBufferToImage(cmd, stagingBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        m_device.endSingleTimeCommands(cmd);
        m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    ImTextureID VulkanTexture::getImGuiTextureID() const {
        // Created on first use: most textures are never shown by ImGui, and the not_found texture is built
        // before ImGui is initialised (ImGui_ImplVulkan_GetTextureDescriptorSetLayout isn't available yet)
        if (m_descriptorSet == VK_NULL_HANDLE)
            m_descriptorSet = createImGuiTextureDescriptor(m_device, m_sampler, m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        return reinterpret_cast<ImTextureID>(m_descriptorSet);
    }

    VulkanTexture::~VulkanTexture() {
        VC_PROFILER_FUNCTION();
        // Wait before freeing anything: a frame still in flight may sample this texture or draw it through ImGui
        // (destroyImage waits too, but only after the descriptor, sampler and view would already be gone)
        vkDeviceWaitIdle(m_device.device());
        destroyImGuiTextureDescriptor(m_device, m_descriptorSet);
        if (m_sampler != VK_NULL_HANDLE)
            vkDestroySampler(m_device.device(), m_sampler, nullptr);
        if (m_imageView != VK_NULL_HANDLE)
            vkDestroyImageView(m_device.device(), m_imageView, nullptr);
        if (m_image != VK_NULL_HANDLE)
            m_device.destroyImage(m_image, m_allocation);
    }
} // Vectrix
