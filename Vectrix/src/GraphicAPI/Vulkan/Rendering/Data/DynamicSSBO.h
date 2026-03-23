#ifndef VECTRIXWORKSPACE_DYNAMICSSBO_H
#define VECTRIXWORKSPACE_DYNAMICSSBO_H
#include "../Core/Device.h"
#include "Vectrix/Rendering/Shaders/ShaderUniformLayout.h"

namespace Vectrix {
    class DynamicSSBO {
    public:
        DynamicSSBO(ShaderUniformLayout* layout, uint32_t initialCapacity = 256);
        ~DynamicSSBO();

        void write(uint32_t frameIndex, uint32_t elementIndex, const void* src);
        void flush(uint32_t frameIndex) const;
        void reset(uint32_t frameIndex);

        [[nodiscard]] VkDescriptorSet descriptorSet(uint32_t frameIndex) const { return m_descriptorSets[frameIndex]; }
        [[nodiscard]] uint32_t capacity() const { return m_capacity; }

        [[nodiscard]] std::uint32_t getSetCountID() const { return m_setCountID; }
        static VkDescriptorSetLayout getStaticDescriptorSetLayout() {
            if (s_descriptorSetLayout==nullptr) {
                VC_CORE_TRACE("Creating DynamicSSBO descriptorSetLayout");
                createDescriptorSetLayout();
            }
            return s_descriptorSetLayout;
        }
    private:
        static VkDescriptorSetLayout s_descriptorSetLayout;
        static void createDescriptorSetLayout();


        void allocateGPUBuffer();
        void updateDescriptorSets() const;
        void grow();

        Device& m_device;
        ShaderUniformLayout* m_layout;

        uint32_t m_elementStride{};

        uint32_t m_capacity{};

        uint32_t m_framesInFlight{};

        VkBuffer m_buffer{};
        VmaAllocation m_allocation{};
        void* m_mapped{};

        std::vector<uint8_t> m_storage;

        std::vector<VkDescriptorSet> m_descriptorSets;
        std::uint32_t m_setCountID;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_DYNAMICSSBO_H