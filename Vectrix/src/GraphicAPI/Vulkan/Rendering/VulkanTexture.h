#ifndef VECTRIXWORKSPACE_VULKANTEXTURE_H
#define VECTRIXWORKSPACE_VULKANTEXTURE_H
#include "Core/Device.h"
#include "stb_image.h"
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
            info.imageLayout = m_layout;
            info.imageView = m_imageView;
            info.sampler = m_sampler;
            return info;
        }

        [[nodiscard]] VkFormat getFormat() const { return m_format; }
        [[nodiscard]] VkImageLayout getLayout() const { return m_layout; }

        [[nodiscard]] uint32_t getUniqueTextureID() { return m_id; }
    private:
        friend class Texture;
        VulkanTexture();
        void createTexture(stbi_uc* pixels, int channels);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        int m_width = -1;
        int m_height = -1;
        int m_channel = -1;
        VkDeviceSize m_imageSize;
        VkImage m_image = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;
        VkFormat m_format = VK_FORMAT_UNDEFINED;
        VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        Device& m_device;
        uint32_t m_id;

        static uint32_t s_numberTexture;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANTEXTURE_H