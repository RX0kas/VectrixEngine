#ifndef VECTRIXWORKSPACE_VULKANFRAMEBUFFER_H
#define VECTRIXWORKSPACE_VULKANFRAMEBUFFER_H
#include "imgui.h"
#include "Core/Device.h"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Rendering/Framebuffer.h"

namespace Vectrix {
    class VulkanFramebuffer : public Framebuffer {
    public:
        VulkanFramebuffer(const FramebufferSpecification& spec);
        ~VulkanFramebuffer() override;

        void bind() override;
        void unbind() override;

        [[nodiscard]] ImTextureID getTextureID() const override {
            if (isBound()) {
                VC_CORE_ERROR("Trying to get the texture of a framebuffer that is not finished\nTry calling framebuffer.unbind()");
            }
            return reinterpret_cast<ImTextureID>(m_descriptorSet);
        }

        void resize(glm::vec2 size) override;

        [[nodiscard]] const FramebufferSpecification& getSpecification() const override { return m_specification; }
        [[nodiscard]] VkExtent2D getExtent() const { return {m_specification.width, m_specification.height}; }
        [[nodiscard]] VkImage getImage() const { return m_image; }
        [[nodiscard]] VkImageView getImageView() const { return m_imageView; }
        [[nodiscard]] VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }
        [[nodiscard]] bool isBound() const override { return m_bind; }
        [[nodiscard]] float getAspectRatio() const override { return static_cast<float>(m_specification.width)/static_cast<float>(m_specification.height);}
    private:
        static void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        Device& m_device;

        FramebufferSpecification m_specification;
        VkImage m_image = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        VkImage m_depthImage = VK_NULL_HANDLE;
        VkImageView m_depthImageView = VK_NULL_HANDLE;
        VmaAllocation  m_depthAllocation = VK_NULL_HANDLE;
        VkFormat m_depthFormat;

        VkSampler m_sampler = VK_NULL_HANDLE;
        VkDescriptorSet m_descriptorSet;
        VkImageLayout m_currentLayout;
        VkImageLayout m_currentDepthLayout;

        bool m_bind = false;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANFRAMEBUFFER_H