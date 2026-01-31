#include "VulkanShader.h"

#include <utility>


namespace Vectrix {
	VulkanShader::VulkanShader(std::string name, const std::string& vertexPath, const std::string& fragmentPath,const ShaderUniformLayout& layout, BufferLayout buffer_layout)
		: m_device(VulkanContext::instance().getDevice()), m_renderer(VulkanContext::instance().getRenderer()), m_layout(std::make_unique<ShaderUniformLayout>(layout)), m_name{std::move(name)}
	{
		m_layout->finalize();
		m_ssbo = std::make_unique<SSBO>(m_device,*m_layout);
		vkDeviceWaitIdle(m_device.device());
		createPipelineLayout();
		vkDeviceWaitIdle(m_device.device());
		createPipeline(m_renderer.getSwapChainRenderPass(), vertexPath, fragmentPath, std::move(buffer_layout));
	}

    VulkanShader::~VulkanShader()
	{
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	void VulkanShader::bind() const {
		m_pipeline->bind(m_renderer.getCurrentCommandBuffer());
		int currentFrame = m_renderer.getFrameIndex();
		m_ssbo->uploadFrame(currentFrame, m_ssbo->framePtr(currentFrame));
		VkDescriptorSet ds = m_ssbo->descriptorSet(currentFrame);
		vkCmdBindDescriptorSets(m_renderer.getCurrentCommandBuffer(),VK_PIPELINE_BIND_POINT_GRAPHICS,m_pipelineLayout, 0, 1, &ds, 0, nullptr);
	}

	void VulkanShader::unbind() const
	{

	}

	void VulkanShader::setUniformBool(const std::string &name, bool value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Bool) {
			VC_CORE_ERROR("{} is not a boolean in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(bool));
	}

	void VulkanShader::setUniform1i(const std::string &name, int value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Int) {
			VC_CORE_ERROR("{} is not an integer in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(int));
	}

	void VulkanShader::setUniform1u(const std::string &name, unsigned int value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Uint) {
			VC_CORE_ERROR("{} is not an unsigned integer in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(unsigned int));
	}

	void VulkanShader::setUniform1f(const std::string &name, float value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Float) {
			VC_CORE_ERROR("{} is not a float in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(float));
	}

	void VulkanShader::setUniform2f(const std::string &name, glm::vec2 value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Vec2) {
			VC_CORE_ERROR("{} is not a vector 2 in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(glm::vec2));
	}

	void VulkanShader::setUniform3f(const std::string &name, glm::vec3 value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Vec3) {
			VC_CORE_ERROR("{} is not a vector 3 in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(glm::vec3));
	}

	void VulkanShader::setUniform4f(const std::string &name, glm::vec4 value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Vec4) {
			VC_CORE_ERROR("{} is not a vector 4 in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(glm::vec4));
	}

	void VulkanShader::setUniformMat4f(const std::string &name, glm::mat4 value) const {
		VC_VERIFY_UNIFORM_NAME(name);
		auto* e = m_layout->find(name);
		if (e==nullptr) {
			VC_CORE_ERROR("{} is not found in the layout",name);
		}
		if (e->type!=ShaderUniformType::Mat4) {
			VC_CORE_ERROR("{} is not a matrice 4 in the buffer layout",name);
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &value, sizeof(glm::mat4));
	}

	void VulkanShader::sentCameraUniform(const PerspectiveCamera &camera) const {
		auto* e = m_layout->find("cameraTransform");
		if (e == nullptr) {
			VC_CORE_ERROR("cameraTransform is not found in the layout");
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &camera.getTransformationMatrix(), sizeof(glm::mat4));
	}

	void VulkanShader::setModelMatrix(const glm::mat4& model) const {
		auto* e = m_layout->find("modelMat");
		if (e == nullptr) {
			VC_CORE_ERROR("modelMat is not found in the layout");
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &model, sizeof(glm::mat4));
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
		VkDescriptorSetLayout dsl = m_ssbo->descriptorSetLayout();

		if (dsl == VK_NULL_HANDLE) {
			VC_CORE_ERROR("Descriptor set layout is null!");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &dsl;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VkResult result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS) {
			VC_CORE_CRITICAL("Failed to create pipeline layout: {}", result);
		}
	}
} // Vectrix