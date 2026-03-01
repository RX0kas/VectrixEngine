#include "VulkanShader.h"

#include <fstream>
#include <utility>

#include "Vectrix/Application.h"

#define OPTIMIZE

namespace Vectrix {
	VulkanShader::VulkanShader(std::string name, const std::string& vertexPath, const std::string& fragmentPath,const ShaderUniformLayout& layout, BufferLayout buffer_layout,bool affectedByCamera)
		: m_device(VulkanContext::instance().getDevice()), m_renderer(VulkanContext::instance().getRenderer()), m_layout(std::make_unique<ShaderUniformLayout>(layout)), m_name{std::move(name)},m_affectedByCamera(affectedByCamera)
	{
		finalize(m_layout.get());
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

	void VulkanShader::sendCameraUniform(const glm::mat4& camera) const {
		auto* e = m_layout->find("vc_cameraTransform");
		if (e == nullptr) {
			VC_CORE_ERROR("Sending camera uniform in a shader that doesn't support camera");
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &camera, sizeof(glm::mat4));
	}
	void VulkanShader::setModelMatrix(const glm::mat4& model) const {
		vkCmdPushConstants(m_renderer.getCurrentCommandBuffer(), m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4), &model);
	}

	void VulkanShader::setTexture(uint32_t index, Ref<Texture> texture) {
		VC_CORE_ASSERT(index < Texture::getMaxTexture(), "Index out of range");
		auto vkTex = std::dynamic_pointer_cast<VulkanTexture>(texture);
		auto textures = m_ssbo->textures();
		textures[index] = vkTex;

		VkDescriptorImageInfo imageInfo = vkTex->getDescriptorInfo();
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = m_ssbo->descriptorSet()[VulkanContext::instance().getRenderer().getFrameIndex()];
		write.dstBinding = 1;
		write.dstArrayElement = index;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);

		vkCmdPushConstants(m_renderer.getCurrentCommandBuffer(), m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(unsigned int) , &index);
	}

	std::string readUTF8(const std::string& path) {
		std::ifstream fichier(path, std::ios::binary);
		if (!fichier.is_open()) {
			VC_CORE_ERROR("Can't open: {}", path);
		}
		std::stringstream buffer;
		buffer << fichier.rdbuf();
		return buffer.str();
	}

	void VulkanShader::createPipeline(VkRenderPass renderPass, const std::string& vertexPath, const std::string& fragmentPath,BufferLayout layout) {
		VC_CORE_ASSERT(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		pipelineConfig.layout = std::move(layout);

		VulkanShaderCompiler &compiler = VulkanContext::instance().getCompiler();
		m_vertSRC = readUTF8(vertexPath);
		m_fragSRC = readUTF8(fragmentPath);
		bool optimize = false;
#ifdef OPTIMIZE
		optimize=true;
#endif
		auto vertCode = compiler.compile_file(m_name.c_str(),Vertex_Shader,m_vertSRC.c_str(),optimize);
		auto fragCode = compiler.compile_file(m_name.c_str(),Fragment_Shader,m_fragSRC.c_str(),optimize);

		m_pipeline = std::make_unique<Pipeline>(m_device,vertCode,fragCode,pipelineConfig);
	}

	void VulkanShader::createPipelineLayout() {
		VkDescriptorSetLayout dsl = m_ssbo->descriptorSetLayout();

		if (dsl == VK_NULL_HANDLE) {
			VC_CORE_ERROR("Descriptor set layout is null!");
		}

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4)+sizeof(unsigned int);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &dsl;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VkResult result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS) {
			VC_CORE_CRITICAL("Failed to create pipeline layout: {}", string_VkResult(result));
		}
	}
} // Vectrix