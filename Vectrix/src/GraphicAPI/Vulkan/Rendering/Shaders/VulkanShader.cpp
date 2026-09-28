#include "VulkanShader.h"

#include <cstdint>
#include <utility>

#include "Pipeline.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
	VulkanShader::VulkanShader(std::string name, const std::string& source,const ShaderUniformLayout& layout, BufferLayout buffer_layout,bool affectedByCamera)
		: m_device(VulkanContext::instance().getDevice()), m_renderer(VulkanContext::instance().getRenderer()), m_layout(std::make_unique<ShaderUniformLayout>(layout)), m_affectedByCamera(affectedByCamera),m_name{std::move(name)}
	{
		VC_PROFILER_FUNCTION();
		finalize(m_layout.get());
		m_ssbo = std::make_unique<ShaderSSBO>(m_device,*m_layout);
		createPipelineLayout();
		auto src = parseSource(source);
		m_vertSRC = src.first;
		m_fragSRC = src.second;
		createPipeline(std::move(buffer_layout));
	}

    VulkanShader::~VulkanShader() {
		VC_PROFILER_FUNCTION();
		m_ssbo.reset();
		m_pipeline.reset();
		m_layout.reset();
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	void VulkanShader::bind() const {
		VC_PROFILER_FUNCTION();
		m_pipeline->bind(m_renderer.getCurrentCommandBuffer());
		int currentFrame = m_renderer.getFrameIndex();
		m_ssbo->uploadFrame(currentFrame, m_ssbo->framePtr(currentFrame));
		VkDescriptorSet ds = m_ssbo->descriptorSet(currentFrame);
		vkCmdBindDescriptorSets(m_renderer.getCurrentCommandBuffer(),VK_PIPELINE_BIND_POINT_GRAPHICS,m_pipelineLayout, m_ssbo->getSetCountID(), 1, &ds, 0, nullptr);
	}

	// The typed setters all go through setUniformImplementation, which checks the name and type and
	// returns on a mismatch (VC_CORE_ERROR is compiled out in release, so it must not fall through)
	void VulkanShader::setUniformBool(const std::string &name, bool value) const {
		setUniformImplementation(name, ShaderUniformType::Bool, &value, sizeof(bool));
	}

	void VulkanShader::setUniform1i(const std::string &name, int value) const {
		setUniformImplementation(name, ShaderUniformType::Int, &value, sizeof(int));
	}

	void VulkanShader::setUniform1u(const std::string &name, unsigned int value) const {
		setUniformImplementation(name, ShaderUniformType::Uint, &value, sizeof(unsigned int));
	}

	void VulkanShader::setUniform1f(const std::string &name, float value) const {
		setUniformImplementation(name, ShaderUniformType::Float, &value, sizeof(float));
	}

	void VulkanShader::setUniform2f(const std::string &name, glm::vec2 value) const {
		setUniformImplementation(name, ShaderUniformType::Vec2, &value, sizeof(glm::vec2));
	}

	void VulkanShader::setUniform3f(const std::string &name, glm::vec3 value) const {
		setUniformImplementation(name, ShaderUniformType::Vec3, &value, sizeof(glm::vec3));
	}

	void VulkanShader::setUniform4f(const std::string &name, glm::vec4 value) const {
		setUniformImplementation(name, ShaderUniformType::Vec4, &value, sizeof(glm::vec4));
	}

	void VulkanShader::setUniformMat4f(const std::string &name, glm::mat4 value) const {
		setUniformImplementation(name, ShaderUniformType::Mat4, &value, sizeof(glm::mat4));
	}

	void VulkanShader::sendCameraUniform(const glm::mat4& camera) const {
		VC_PROFILER_FUNCTION();
		auto* e = m_layout->find("vc_cameraTransform");
		if (e == nullptr) {
			VC_CORE_ERROR("Sending camera uniform in a shader that doesn't support camera");
			return;
		}
		m_ssbo->copyToFrame(m_renderer.getFrameIndex(), e->offset, &camera, sizeof(glm::mat4));
	}

	uint32_t VulkanShader::useTexture(std::shared_ptr<Texture> texture) {
		VC_PROFILER_FUNCTION();
		auto vkTex = std::dynamic_pointer_cast<VulkanTexture>(texture);
		VC_CORE_ASSERT(vkTex != nullptr, "Texture used by shader '{}' is not a VulkanTexture", m_name);
		return useImage("texture:" + std::to_string(vkTex->getUniqueTextureID()), vkTex->getDescriptorInfo());
	}

	uint32_t VulkanShader::useFramebuffer(std::shared_ptr<Framebuffer> framebuffer) {
		VC_PROFILER_FUNCTION();
		auto vkFramebuffer = std::dynamic_pointer_cast<VulkanFramebuffer>(framebuffer);
		VC_CORE_ASSERT(vkFramebuffer != nullptr, "Framebuffer used by shader '{}' is not a VulkanFramebuffer", m_name);
		VC_CORE_ASSERT(!vkFramebuffer->isBound(), "Framebuffer used by shader '{}' must be unbound before sampling", m_name);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = vkFramebuffer->getImageLayout();
		imageInfo.imageView = vkFramebuffer->getImageView();
		imageInfo.sampler = vkFramebuffer->getSampler();

		// Keyed on the framebuffer object, not its image view: resize() recreates the view, and a new key would
		// take a new array slot while the shader keeps sampling the old slot (now pointing at a destroyed view).
		// useImage rewrites the slot on every call, so the current view is always the one bound.
		const auto key = "framebuffer:" + std::to_string(reinterpret_cast<std::uintptr_t>(vkFramebuffer.get()));
		return useImage(key, imageInfo);
	}

	uint32_t VulkanShader::useImage(const std::string& key, const VkDescriptorImageInfo& imageInfo) {
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = m_ssbo->descriptorSet(m_renderer.getFrameIndex());
		write.dstBinding = 1;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;

		if (const auto it = m_imageIndexCache.find(key); it != m_imageIndexCache.end()) {
			write.dstArrayElement = it->second;
			vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
			return it->second;
		}

		if (m_firstTextureIndexAvailable >= Texture::getMaxTexturePerShader()) {
			// Writing past the u_Textures array is invalid: fall back to slot 0 (the not_found texture until something else is bound there)
			VC_CORE_ERROR_NO_EXIT("Too many texture/framebuffer images have been set in the shader {} (max {})", m_name, Texture::getMaxTexturePerShader());
			return 0;
		}
		write.dstArrayElement = m_firstTextureIndexAvailable;
		vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
		m_imageIndexCache.emplace(key, m_firstTextureIndexAvailable);
		return m_firstTextureIndexAvailable++;
	}

	void VulkanShader::createPipeline(BufferLayout layout) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		pipelineConfig.layout = std::move(layout);

		if (m_name.ends_with("mask.vcshader")) {
			pipelineConfig.overrideVertexInput = true;
			pipelineConfig.bindingDescriptions = getVertexBindingDescriptions(pipelineConfig.layout);
			pipelineConfig.attributeDescriptions = getVertexAttributeDescriptions(pipelineConfig.layout);
			pipelineConfig.attributeDescriptions.resize(1);
		} else if (m_name.ends_with("outline.vcshader")) {
			pipelineConfig.overrideVertexInput = true;
			pipelineConfig.bindingDescriptions.clear();
			pipelineConfig.attributeDescriptions.clear();
			pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
			pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
			pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
		}

		VulkanShaderCompiler &compiler = VulkanContext::instance().getCompiler();
		const bool optimize = VulkanContext::settings().shaders.optimize;
		auto vertCode = compiler.compile_file(m_name.c_str(),VertexShader,m_vertSRC.c_str(),optimize);
		auto fragCode = compiler.compile_file(m_name.c_str(),FragmentShader,m_fragSRC.c_str(),optimize);
		if (vertCode.empty() || fragCode.empty())
			return; // No pipeline: Shader::createFromSource reports the shader as failed

		m_pipeline = std::make_unique<Pipeline>(m_device,vertCode,fragCode,pipelineConfig);
	}

	void VulkanShader::createPipelineLayout() {
		VC_PROFILER_FUNCTION();
		VkDescriptorSetLayout dsl = m_ssbo->descriptorSetLayout();

		if (dsl == VK_NULL_HANDLE) {
			VC_CORE_ERROR("Descriptor set layout is null!");
		}

		std::array<VkDescriptorSetLayout, 2> layouts = { dsl, DynamicSSBO::getStaticDescriptorSetLayout() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VkResult result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS) {
			VC_CORE_CRITICAL("Failed to create pipeline layout: {}", string_VkResult(result));
		}
	}
} // Vectrix
