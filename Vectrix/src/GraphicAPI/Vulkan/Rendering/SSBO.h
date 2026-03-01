#ifndef VECTRIXWORKSPACE_SSBO_H
#define VECTRIXWORKSPACE_SSBO_H
#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "GraphicAPI/Vulkan/Rendering/SwapChain.h"
#include <vector>

#include "VulkanTexture.h"
#include "Vectrix/Rendering/Buffer.h"
#include "Vectrix/Rendering/Shaders/ShaderUniformLayout.h"

namespace Vectrix {

    class SSBO {
    public:
        SSBO(Device& device, ShaderUniformLayout& layout);

        ~SSBO();

        void uploadFrame(uint32_t frameIndex, const void* src) {
            uint8_t* dst = static_cast<uint8_t*>(m_mapped) + static_cast<size_t>(frameIndex) * m_elementStride;
            std::memcpy(dst, src, m_elementStride);
        }

        void copyToFrame(uint32_t frameIndex, const uint32_t offset, const void* src, const size_t s) {
            VC_CORE_ASSERT(frameIndex < m_framesInFlight,"FrameIndex ({}) too high for the number of frames ({})",frameIndex,m_framesInFlight);
            uint8_t* dst = m_storage.data() + static_cast<size_t>(frameIndex) * m_elementStride + offset;
            std::memcpy(dst, src, s);
        }

        // pointer to the CPU buffer for a frame (used to upload to mapped GPU memory)
        [[nodiscard]] void const* framePtr(const uint32_t frameIndex) const {
            return m_storage.data() + static_cast<size_t>(frameIndex) * m_elementStride;
        }

        [[nodiscard]] VkDescriptorSet descriptorSet(uint32_t frameIndex) const { return m_descriptorSets[frameIndex]; }
        [[nodiscard]] std::vector<VkDescriptorSet> descriptorSet() const { return m_descriptorSets; }
        [[nodiscard]] VkDescriptorSetLayout descriptorSetLayout() const { return m_descriptorSetLayout; }
        [[nodiscard]] VkDeviceSize elementStride() const { return m_elementStride; }
        [[nodiscard]] std::array<Ref<VulkanTexture>,Texture::getMaxTexture()> textures() const {return m_textures;}
    private:
        void createDescriptorSetLayout();
        Device& m_device;
        VkBuffer m_buffer{};
        void* m_mapped{};
        VkDescriptorSetLayout m_descriptorSetLayout{};
        std::vector<VkDescriptorSet> m_descriptorSets;
        VkDeviceSize m_bufferSize{};
        uint32_t m_elementStride{};
        uint32_t m_framesInFlight{};
        std::vector<uint8_t> m_storage;
        ShaderUniformLayout& m_layout;

        std::array<Ref<VulkanTexture>, Texture::getMaxTexture()> m_textures;

        VmaAllocation m_allocation;
    };
} // Vectrix



#endif //VECTRIXWORKSPACE_SSBO_H