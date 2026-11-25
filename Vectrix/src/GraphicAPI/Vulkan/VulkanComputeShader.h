#ifndef VECTRIXWORKSPACE_VULKANCOMPUTESHADER_H
#define VECTRIXWORKSPACE_VULKANCOMPUTESHADER_H
#include <array>
#include <memory>
#include <string>

#include "Data/ComputePipeline.h"
#include "Vectrix/Physics/ComputeShader.h"

namespace Vectrix {
	class VulkanComputeShader : public ComputeShader {
	public:
		VulkanComputeShader(const std::string& name, const std::string& source);
		/// The pipeline, its layout and descriptor set are destroyed once no frame in flight may use them
		~VulkanComputeShader() override;

		bool setBuffer(uint32_t binding, const void* data, size_t size) override;
		bool allocateBuffer(uint32_t binding, size_t size) override;
		[[nodiscard]] size_t getBufferSize(uint32_t binding) const override;
		bool setPushConstants(const void* data, size_t size) override;
		bool dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;
		bool readBuffer(uint32_t binding, void* out, size_t size, size_t offset) const override;

		using ComputeShader::setBuffer;
		using ComputeShader::setPushConstants;
		using ComputeShader::readBuffer;

		[[nodiscard]] const std::string& getName() const { return m_name; }
	private:
		friend class ComputeShader;
		/// A storage buffer the CPU writes and reads directly (host visible, mapped)
		struct Buffer {
			VkBuffer buffer = VK_NULL_HANDLE;
			VmaAllocation allocation = VK_NULL_HANDLE;
			void* mapped = nullptr;
			size_t size = 0;
		};

		[[nodiscard]] bool isValid() const { return m_computePipeline && m_computePipeline->isValid() && m_descriptorSet != VK_NULL_HANDLE; }
		void createLayouts();
		void createDescriptorSet();
		/// Make sure the binding has a buffer of exactly size bytes (a new one loses the previous content)
		bool ensureBuffer(uint32_t binding, size_t size);
		void destroyBuffer(Buffer& buffer) const;

		Device& m_device;
		std::string m_name;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
		std::unique_ptr<ComputePipeline> m_computePipeline;
		std::array<Buffer, k_maxBuffers> m_buffers{};
		std::array<std::byte, k_maxPushConstantSize> m_pushConstants{};
		size_t m_pushConstantSize = 0;
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANCOMPUTESHADER_H
