#include "VulkanFramebuffer.h"

#include "volk.h"
#include "GraphicAPI/Vulkan/ImGui/imgui_impl_vulkan.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
    static VkDescriptorSet createImGuiTextureDescriptor(Device& device, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout) {
        VkDescriptorSetLayout layout = ImGui_ImplVulkan_GetTextureDescriptorSetLayout();

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = device.descriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
            VC_CORE_ERROR("Failed to allocate ImGui framebuffer descriptor set");
            return VK_NULL_HANDLE;
        }

        ImGui_ImplVulkan_WriteTextureDescriptor(descriptorSet, sampler, imageView, imageLayout);
        return descriptorSet;
    }

    static void destroyImGuiTextureDescriptor(Device& device, VkDescriptorSet descriptorSet) {
        if (descriptorSet == VK_NULL_HANDLE) return;
        vkFreeDescriptorSets(device.device(), device.descriptorPool(), 1, &descriptorSet);
    }

    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification &spec) : m_device(VulkanContext::instance().getDevice()), m_specification(spec) {
        createResources();
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        destroyResources();
    }

    void VulkanFramebuffer::createResources() {
        // Formats left undefined follow the swap chain, and are written back so resize() reuses them
        const VkFormat format = m_specification.imageFormat != UNDEFINED ? toVulkanFormat(m_specification.imageFormat) : VulkanContext::instance().getRenderer().getImageFormat();
        m_specification.imageFormat = toVectrixFormat(format);

        // Color
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.extent = {m_specification.width,m_specification.height,1};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,m_image, m_allocation);

        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView);

        // Depth
        if (m_specification.hasDepth) {
            const VkFormat depthFormat = m_specification.depthFormat != UNDEFINED ? toVulkanFormat(m_specification.depthFormat) : VulkanContext::instance().getRenderer().findDepthFormat();
            m_specification.depthFormat = toVectrixFormat(depthFormat);

            VkImageCreateInfo depthImageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
            depthImageInfo.format = depthFormat;
            depthImageInfo.extent = { m_specification.width, m_specification.height, 1 };
            depthImageInfo.mipLevels = 1;
            depthImageInfo.arrayLayers = 1;
            depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            m_device.createImageWithInfo(depthImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthAllocation);

            VkImageViewCreateInfo depthViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            depthViewInfo.image = m_depthImage;
            depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthViewInfo.format = depthFormat;
            depthViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
            vkCreateImageView(m_device.device(), &depthViewInfo, nullptr, &m_depthImageView);
        }

        VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler);

        m_descriptorSet = createImGuiTextureDescriptor(m_device, m_sampler, m_imageView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_currentDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    void VulkanFramebuffer::destroyResources() {
        // A frame still in flight may render into these or show them through ImGui: they go once it's done, rather
        // than waiting for the whole GPU (which every viewport resize used to do)
        VulkanContext::destroyWhenUnused([device = &m_device, descriptorSet = m_descriptorSet, sampler = m_sampler,
                                          imageView = m_imageView, image = m_image, allocation = m_allocation,
                                          depthImageView = m_depthImageView, depthImage = m_depthImage, depthAllocation = m_depthAllocation] {
            destroyImGuiTextureDescriptor(*device, descriptorSet);
            vkDestroySampler(device->device(), sampler, nullptr);
            vkDestroyImageView(device->device(), imageView, nullptr);
            if (depthImage != VK_NULL_HANDLE) {
                vkDestroyImageView(device->device(), depthImageView, nullptr);
                device->destroyImage(depthImage, depthAllocation);
            }
            device->destroyImage(image, allocation);
        });
        m_descriptorSet = VK_NULL_HANDLE;
        m_sampler = VK_NULL_HANDLE;
        m_imageView = VK_NULL_HANDLE;
        m_image = VK_NULL_HANDLE;
        m_depthImageView = VK_NULL_HANDLE;
        m_depthImage = VK_NULL_HANDLE;
    }

    void VulkanFramebuffer::bind(bool clear) {
        bind(clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD);
    }

    void VulkanFramebuffer::bind(VkAttachmentLoadOp loadOp) {
        VC_CORE_ASSERT(!m_bind,"Framebuffer already bound");
        VC_CORE_ASSERT(s_currentFramebuffer==nullptr,"Another framebuffer is already bound");
        VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
        VC_CORE_ASSERT(cmd != VK_NULL_HANDLE, "No active command buffer!");

        s_currentFramebuffer = this;

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
        preBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        vkCmdPipelineBarrier(cmd, srcStage, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,0, nullptr, 0, nullptr, 1, &preBarrier);

        if (m_specification.hasDepth) {
            // Once used, the depth image still holds the previous pass' writes: order them before this
            // pass' clear/writes (a TOP_OF_PIPE/no-access source would leave a write-after-write hazard)
            const bool firstUse = m_currentDepthLayout == VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageMemoryBarrier depthBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            depthBarrier.image = m_depthImage;
            depthBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
            depthBarrier.oldLayout = m_currentDepthLayout;
            depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthBarrier.srcAccessMask = firstUse ? 0 : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            const VkPipelineStageFlags depthStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            vkCmdPipelineBarrier(cmd, firstUse ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : depthStages, depthStages,
                                 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);
            m_currentDepthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkRenderingAttachmentInfoKHR colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
        colorAttachment.imageView = m_imageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = loadOp;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        if (m_specification.clearColor) {
            const glm::vec4& c = *m_specification.clearColor;
            colorAttachment.clearValue.color = { { c.r, c.g, c.b, c.a } };
        } else {
            colorAttachment.clearValue = VulkanContext::instance().getRenderer().getClearValue();
        }

        VkRenderingAttachmentInfoKHR depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
        depthAttachment.imageView = m_depthImageView;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = loadOp;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.clearValue.depthStencil.depth = 1.0f;
        depthAttachment.clearValue.depthStencil.stencil = 0;

        VkRenderingInfoKHR renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
        renderingInfo.renderArea = { {0,0}, {m_specification.width, m_specification.height} };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = m_specification.hasDepth ? &depthAttachment : nullptr;
        vkCmdBeginRenderingKHR(cmd, &renderingInfo);

        m_bind = true;
        m_currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    void VulkanFramebuffer::unbind() {
        if (!m_bind) {
            VC_CORE_ERROR("Framebuffer not bound");
        }
        if (s_currentFramebuffer!=this) {
            VC_CORE_ERROR("Can't unbind a framebuffer because another is bound");
        }
        s_currentFramebuffer = nullptr;
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

    void VulkanFramebuffer::resize(glm::vec2 size) {
        VC_CORE_ASSERT(!m_bind, "Cannot resize a bound framebuffer, call unbind() first!");
        const auto newWidth  = static_cast<uint32_t>(size.x);
        const auto newHeight = static_cast<uint32_t>(size.y);

        if (m_specification.width == newWidth && m_specification.height == newHeight) return;
        VC_CORE_ASSERT(newWidth > 0 && newHeight > 0, "Framebuffer size must be greater than 0!");

        destroyResources();
        m_specification.width  = newWidth;
        m_specification.height = newHeight;
        createResources();
    }

    void VulkanFramebuffer::updateSpecification(FramebufferSpecification &spec) {
        VC_CORE_ASSERT(!m_bind, "Cannot update a bound framebuffer, call unbind() first!");
        // Everything is rebuilt: the formats or the depth attachment may change, not only the size
        destroyResources();
        m_specification = spec;
        createResources();
    }
} // Vectrix
