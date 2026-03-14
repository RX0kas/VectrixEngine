#ifndef VECTRIXWORKSPACE_DYNAMICSSBO_H
#define VECTRIXWORKSPACE_DYNAMICSSBO_H
#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "Vectrix/Rendering/Shaders/ShaderUniformLayout.h"

namespace Vectrix {
    class DynamicSSBO {
    public:
        DynamicSSBO(Device& device, ShaderUniformLayout& layout, uint32_t initialCapacity = 256);
        ~DynamicSSBO();

        void write(uint32_t frameIndex, uint32_t elementIndex, const void* src);
        void flush(uint32_t frameIndex) const;
        void reset(uint32_t frameIndex);

        [[nodiscard]] VkDescriptorSet descriptorSet(uint32_t frameIndex) const { return m_descriptorSets[frameIndex]; }
        [[nodiscard]] VkDescriptorSetLayout descriptorSetLayout() const { return m_descriptorSetLayout; }
        [[nodiscard]] uint32_t capacity() const { return m_capacity; }

    private:
        void allocateGPUBuffer();
        void createDescriptorSetLayout();
        void updateDescriptorSets() const;
        void grow();

        Device& m_device;
        ShaderUniformLayout& m_layout;

        uint32_t m_elementStride{};

        uint32_t m_capacity{};

        uint32_t m_framesInFlight{};

        VkBuffer m_buffer{};
        VmaAllocation m_allocation{};
        void* m_mapped{};

        std::vector<uint8_t> m_storage;

        VkDescriptorSetLayout m_descriptorSetLayout{};
        std::vector<VkDescriptorSet> m_descriptorSets;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_DYNAMICSSBO_H