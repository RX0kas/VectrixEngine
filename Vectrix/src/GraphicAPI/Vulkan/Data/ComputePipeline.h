#ifndef VECTRIXWORKSPACE_COMPUTEPIPELINE_H
#define VECTRIXWORKSPACE_COMPUTEPIPELINE_H
#include <cstdint>
#include <string>
#include <vector>
#include "GraphicAPI/Vulkan/Core/Device.h"

namespace Vectrix {
	/// A compute pipeline built from SPIR-V. The layout belongs to the caller (VulkanComputeShader), which has to
	/// keep it alive as long as the pipeline. Can be built on any thread (Device::pipelineCache is synchronized)
	class ComputePipeline {
	public:
		/// @param computeCode The SPIR-V, not empty
		/// @param layout The pipeline layout (descriptor set layouts, push constants)
		/// @param name For the logs and GPU debuggers
		ComputePipeline(const std::vector<uint32_t>& computeCode, VkPipelineLayout layout, const std::string& name);
		/// Destroys the pipeline right away: whoever owns it defers this until the GPU is done (destroyWhenUnused)
		~ComputePipeline();
		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer) const;

		/// False when the pipeline couldn't be created (the error is logged)
		[[nodiscard]] bool isValid() const { return m_computePipeline != VK_NULL_HANDLE; }
		[[nodiscard]] VkPipeline getPipeline() const { return m_computePipeline; }
	private:
		Device& m_device;
		VkPipeline m_computePipeline = VK_NULL_HANDLE;
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_COMPUTEPIPELINE_H
