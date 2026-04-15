#include "VulkanFramebuffer.h"

#include "volk.h"

#include "backends/imgui_impl_vulkan.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
    VulkanFramebuffer* VulkanFramebuffer::s_currentFB = nullptr;


    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification &spec) : m_device(VulkanContext::instance().getDevice()),m_specification(spec), m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)  {
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VulkanContext::instance().getRenderer().getImageFormat();
        imageInfo.extent = {spec.width,spec.height,1};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,m_image, m_allocation);

        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VulkanContext::instance().getRenderer().getImageFormat();
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView);

        VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler);

        m_descriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_imageView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        ImGui_ImplVulkan_RemoveTexture(m_descriptorSet);
        vkDestroySampler(m_device.device(),m_sampler,nullptr);
        vkDestroyImageView(m_device.device(),m_imageView,nullptr);
        m_device.destroyImage(m_image,m_allocation);
    }

    void VulkanFramebuffer::transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.image = image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        VkPipelineStageFlags srcStage = 0, dstStage = 0;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0,0, nullptr, 0, nullptr, 1, &barrier);
    }

    void VulkanFramebuffer::bind() {
        if (m_bind) {
            VC_CORE_ERROR("Framebuffer already bound");
        }
        if (s_currentFB!=nullptr) {
            VC_CORE_ERROR("Another framebuffer is already bound");
        }
        s_currentFB = this;

        VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
        VC_CORE_ASSERT(cmd != VK_NULL_HANDLE, "No active command buffer!");

        VkImageMemoryBarrier preBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        preBarrier.image = m_image;
        preBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        preBarrier.oldLayout = m_currentLayout;
        preBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkPipelineStageFlags srcStage;
        if (m_currentLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            preBarrier.srcAccessMask = 0;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        } else {
            preBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        preBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0,0, nullptr, 0, nullptr, 1, &preBarrier);


        VkRenderingAttachmentInfoKHR colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
        colorAttachment.imageView = m_imageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        VkRenderingInfoKHR renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
        renderingInfo.renderArea = { {0,0}, {m_specification.width, m_specification.height} };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRenderingKHR(cmd, &renderingInfo);
        m_bind = true;
        m_currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    void VulkanFramebuffer::unbind() {
        if (!m_bind) {
            VC_CORE_ERROR("Framebuffer not bound");
        }
        if (s_currentFB!=this) {
            VC_CORE_ERROR("Can't unbind a framebuffer because another is bound");
        }
        s_currentFB = nullptr;

        VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
        VC_CORE_ASSERT(cmd != VK_NULL_HANDLE, "No active command buffer!");

        vkCmdEndRenderingKHR(cmd);

        VkImageMemoryBarrier postBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        postBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        postBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        postBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        postBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        postBarrier.image = m_image;
        postBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &postBarrier);

        m_bind = false;
        m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
} // Vectrix