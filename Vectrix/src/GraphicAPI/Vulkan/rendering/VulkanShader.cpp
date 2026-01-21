#include "VulkanShader.h"

#include "Vectrix/Renderer/Camera/CameraPush.h"

namespace Vectrix {
	VulkanShader::VulkanShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,ShaderUniformLayout layout, BufferLayout buffer_layout)
		: m_device(VulkanContext::instance().getDevice()), m_renderer(VulkanContext::instance().getRenderer()), m_layout(std::move(layout)), m_name{name.c_str()}
	{
		m_ssbo = std::make_unique<SSBO>(m_device,m_layout);
		createPipelineLayout();
		createPipeline(m_renderer.getSwapChainRenderPass(), vertexPath, fragmentPath, buffer_layout);
	}

    VulkanShader::~VulkanShader()
	{
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	void VulkanShader::bind() const
	{
		m_pipeline->bind(m_renderer.getCurrentCommandBuffer());
		int currentFrame = m_renderer.getFrameIndex();
		m_ssbo->uploadFrame(currentFrame, m_ssbo->framePtr(currentFrame));
		VkDescriptorSet ds = m_ssbo->descriptorSet(currentFrame);
		vkCmdBindDescriptorSets(m_renderer.getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &ds, 0, nullptr);

	}

	void VulkanShader::unbind() const
	{

	}

	void VulkanShader::setUniform1i(const std::string &name, int value) const {
		auto* e = m_layout.find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Int) {
			VC_CORE_ERROR("{} is not a float in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(int));
	}

	void VulkanShader::setUniform1f(const std::string &name, float value) const {
		auto* e = m_layout.find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Float) {
			VC_CORE_ERROR("{} is not a float in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(float));
	}

	void VulkanShader::setUniform2f(const std::string &name, glm::vec2 value) const {
		auto* e = m_layout.find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Vec2) {
			VC_CORE_ERROR("{} is not a float in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(glm::vec2));
	}

	void VulkanShader::setUniform3f(const std::string &name, glm::vec3 value) const {
		auto* e = m_layout.find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Vec3) {
			VC_CORE_ERROR("{} is not a float in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(glm::vec3));
	}

	void VulkanShader::setUniform4f(const std::string &name, glm::vec4 value) const {
		auto* e = m_layout.find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Vec4) {
			VC_CORE_ERROR("{} is not a float in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(glm::vec4));
	}
	void VulkanShader::createPipeline(VkRenderPass renderPass, const std::string& vertexPath, const std::string& fragmentPath,BufferLayout layout) {
		VC_CORE_ASSERT(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		pipelineConfig.layout = std::move(layout);

		m_pipeline = std::make_unique<Pipeline>(
			m_device,
			vertexPath,
			fragmentPath,
			pipelineConfig);
	}

	void VulkanShader::createPipelineLayout() {
		VkPushConstantRange pushConstantRange = CameraPush::createPushConstantRange();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
    	VkDescriptorSetLayout dsl = m_ssbo->descriptorSetLayout();
		pipelineLayoutInfo.pSetLayouts = &dsl;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		VkResult result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS) {
			VC_CORE_CRITICAL("failed to create pipeline layout: {0}",result);
			VC_DEBUGBREAK();
		}
	}
} // Vectrix