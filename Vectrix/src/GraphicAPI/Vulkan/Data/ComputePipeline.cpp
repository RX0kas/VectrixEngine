#include "ComputePipeline.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "GraphicAPI/Vulkan/Core/DebugUtils.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
	ComputePipeline::ComputePipeline(const std::vector<uint32_t>& computeCode, const VkPipelineLayout layout, const std::string& name)
		: m_device(VulkanContext::instance().getDevice()) {
		VC_PROFILER_FUNCTION();
		if (computeCode.empty() || layout == VK_NULL_HANDLE) {
			VC_CORE_ERROR_NO_EXIT("Compute pipeline {}: no SPIR-V or no layout", name);
			return;
		}

		VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		moduleInfo.codeSize = computeCode.size() * sizeof(uint32_t);
		moduleInfo.pCode = computeCode.data();
		VkShaderModule module = VK_NULL_HANDLE;
		if (vkCreateShaderModule(m_device.device(), &moduleInfo, nullptr, &module) != VK_SUCCESS) {
			VC_CORE_ERROR_NO_EXIT("Compute pipeline {}: failed to create the shader module", name);
			return;
		}

		VkComputePipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
		pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		pipelineInfo.stage.module = module;
		pipelineInfo.stage.pName = "main";
		pipelineInfo.layout = layout;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateComputePipelines(m_device.device(), m_device.pipelineCache(), 1, &pipelineInfo, nullptr, &m_computePipeline) != VK_SUCCESS) {
			VC_CORE_ERROR_NO_EXIT("Compute pipeline {}: vkCreateComputePipelines failed", name);
			m_computePipeline = VK_NULL_HANDLE;
		}
		// The pipeline keeps what it needs from the module
		vkDestroyShaderModule(m_device.device(), module, nullptr);

		if (m_computePipeline != VK_NULL_HANDLE)
			DebugUtils::setName(m_device.device(), VK_OBJECT_TYPE_PIPELINE, m_computePipeline, "Compute " + name);
	}

	ComputePipeline::~ComputePipeline() {
		VC_PROFILER_FUNCTION();
		if (m_computePipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(m_device.device(), m_computePipeline, nullptr);
	}

	void ComputePipeline::bind(VkCommandBuffer commandBuffer) const {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(m_computePipeline != VK_NULL_HANDLE, "Compute pipeline is null!");
		VC_CORE_ASSERT(commandBuffer != VK_NULL_HANDLE, "CommandBuffer is null!");
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
	}
} // Vectrix
