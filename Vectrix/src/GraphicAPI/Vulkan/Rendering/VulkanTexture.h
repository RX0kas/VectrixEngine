#ifndef VECTRIXWORKSPACE_VULKANTEXTURE_H
#define VECTRIXWORKSPACE_VULKANTEXTURE_H
#include "Device.h"
#include "Vectrix/Rendering/Textures/Texture.h"

namespace Vectrix {
    class VulkanTexture : public Texture {
    public:
        VulkanTexture(const std::string &path);
        ~VulkanTexture() override;

        [[nodiscard]] uint32_t getWidth() const override { return m_width; }
        [[nodiscard]] uint32_t getHeight() const override { return m_height; }

        [[nodiscard]] VkImageView getImageView() const { return m_imageView; }
        [[nodiscard]] VkSampler getSampler() const { return m_sampler; }
        [[nodiscard]] VkDescriptorImageInfo getDescriptorInfo() const {
            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = m_imageView;
            info.sampler = m_sampler;
            return info;
        }
    private:
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
        int m_width = -1;
        int m_height = -1;
        int m_channel = -1;
        VkDeviceSize m_imageSize;
        VkImage m_image = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;

        Device& m_device;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANTEXTURE_H