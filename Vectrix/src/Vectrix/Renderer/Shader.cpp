#include "vcpch.h"
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	struct SimplePushConstantData {

	};

	Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, BufferLayout layout) : _device(VulkanContext::instance().getDevice())
	{
		createPipelineLayout();
		createPipeline(VulkanContext::instance().getRenderer().getSwapChainRenderPass(), vertexPath, fragmentPath, std::move(layout));
		VulkanContext::instance()._Shaders.push_back(this);
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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr/*&pushConstantRange*/;
		VkResult result = vkCreatePipelineLayout(_device.device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
		if (result != VK_SUCCESS) {
			VC_CORE_CRITICAL("failed to create pipeline layout: {0}",result);
			VC_DEBUGBREAK();
		}
	}

	void Shader::uploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		//GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		//glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}