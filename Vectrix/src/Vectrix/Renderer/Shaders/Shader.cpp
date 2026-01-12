#include "vcpch.h"
#include "Shader.h"

#include <utility>

#include "Vectrix/Renderer/Camera/PerspectiveCamera.h"
#include "Vectrix/Renderer/Shaders/ShaderManager.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	Shader::Shader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, BufferLayout layout) : _device(VulkanContext::instance().getDevice()), name{name.c_str()}
	{
		createPipelineLayout();
		createPipeline(VulkanContext::instance().getRenderer().getSwapChainRenderPass(), vertexPath, fragmentPath, std::move(layout));
	}
	
	Shader::~Shader()
	{
		//_pipeline->cleanup();
		vkDestroyPipelineLayout(_device.device(), _pipelineLayout, nullptr);
	}
	
	void Shader::bind() const
	{
		_pipeline->bind(VulkanContext::instance().getRenderer().getCurrentCommandBuffer());
	}
	
	void Shader::unbind()
	{

	}

	void Shader::createPipeline(VkRenderPass renderPass, const std::string& vertexPath, const std::string& fragmentPath,BufferLayout layout) {
		VC_CORE_ASSERT(_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = _pipelineLayout;
		pipelineConfig.layout = std::move(layout);

		_pipeline = std::make_unique<Pipeline>(
			_device,
			vertexPath,
			fragmentPath,
			pipelineConfig);
	}

	// pushConstantRange.size must be a multiple of 4
	unsigned int roundToFitValidUsage(const unsigned int n) {
		if (n%4==0) return n;
		const float f = floor(n/4);
		return static_cast<unsigned int>(f*4)+4;
	}

	void Shader::createPipelineLayout() {

		/*VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = roundToFitValidUsage(sizeof(SimplePushConstantData));*/

		VkPushConstantRange pushConstantRange = CameraPush::createPushConstantRange();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		VkResult result = vkCreatePipelineLayout(_device.device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
		if (result != VK_SUCCESS) {
			VC_CORE_CRITICAL("failed to create pipeline layout: {0}",result);
			VC_DEBUGBREAK();
		}
	}
}