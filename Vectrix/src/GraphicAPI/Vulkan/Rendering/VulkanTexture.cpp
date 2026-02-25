#include "VulkanTexture.h"

#include <filesystem>

#include "Device.h"
#include "stb_image.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {

    VulkanTexture::VulkanTexture(const std::string &path) : m_device(VulkanContext::instance().getDevice()) {
        stbi_uc* pixels = stbi_load(path.c_str(), &m_width, &m_height, &m_channel, STBI_rgb_alpha);
        m_imageSize = m_width * m_height * 4;

        if (!pixels) {
            VC_CORE_ERROR("Failed to load texture image");
        }
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
        // copy to staging buffer
        void* data;
        vmaMapMemory(m_device.getAllocator(), stagingAllocation, &data);
        memcpy(data, pixels, m_imageSize);
        vmaUnmapMemory(m_device.getAllocator(), stagingAllocation);
        stbi_image_free(pixels);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(m_width);
        imageInfo.extent.height = static_cast<uint32_t>(m_height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Texels are laid out in an implementation defined order for optimal access
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Not usable by the GPU and the very first transition will discard the texels
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        vmaCreateImage(VulkanContext::instance().getAllocator(),&imageInfo,&allocationCreateInfo,&m_image,&m_allocation,nullptr);

        // transition to TRANSFER_DST_OPTIMAL
        transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // staging buffer to image
        m_device.copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height),1);

        // transition to READ_ONLY_OPTIMAL
        transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_device.destroyBuffer(stagingBuffer, stagingAllocation);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
            VC_CORE_ERROR("Failed to create texture image view");
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16.0f;
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

    void VulkanTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const {
        VkCommandBuffer commandBuffer = m_device.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            VC_CORE_ERROR("Unsupported layout transition");
        }

        vkCmdPipelineBarrier(commandBuffer,sourceStage, destinationStage,0,
            0, nullptr, 0, nullptr,1, &barrier);

        m_device.endSingleTimeCommands(commandBuffer);
    }

    VulkanTexture::~VulkanTexture() {
        if (m_sampler != VK_NULL_HANDLE)
            vkDestroySampler(m_device.device(), m_sampler, nullptr);
        if (m_imageView != VK_NULL_HANDLE)
            vkDestroyImageView(m_device.device(), m_imageView, nullptr);
        if (m_image != VK_NULL_HANDLE)
            m_device.destroyImage(m_image, m_allocation);
    }
} // Vectrix