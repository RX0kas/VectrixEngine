#include "vcpch.h"
#include "VulkanContext.h"
#include "Vectrix/Application.h"
#include "Rendering/Mesh/MeshRegistry.h"
#include "Rendering/Mesh/VulkanVertexArray.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"
#include "Vectrix/Settings/SettingsManager.h"

namespace Vectrix {
	VulkanContext* VulkanContext::s_instance = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!s_instance, "VulkanContext already exists!");
		s_instance = this;
		VC_CORE_ASSERT(windowHandle, "Window handle is null!");
		m_compiler = std::make_unique<VulkanShaderCompiler>();
		m_meshRegistry = std::make_unique<MeshRegistry>();
	}

	VulkanContext::~VulkanContext() {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Destroying Graphic context");
		vkDeviceWaitIdle(m_device->device());
		m_renderer->m_batchCache.clear();

		VkDescriptorSetLayout dsl = DynamicSSBO::getStaticDescriptorSetLayout();
		if (dsl!=VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(m_device->device(), dsl, nullptr);

		m_renderer.reset();
		m_compiler.reset();
		m_meshRegistry.reset();

		m_device.reset();
	}

	void VulkanContext::init() {
		VC_PROFILER_FUNCTION();

		m_vkSettings = VulkanSettings::load();

		bool resizable = false;
		if (const JsonObject& s = SettingsManager::getSettings(); s.contains("window"))
			resizable = s.at("window")["resizable"].getAs<bool>().value_or(false);
		glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

		// TODO: Change make it double the size once there is no space left
		const DescriptorPoolConfig cfg {
			m_vkSettings.descriptorPool.uboCount,
			m_vkSettings.descriptorPool.ssboCount,
			m_vkSettings.descriptorPool.samplerCount,
			m_vkSettings.descriptorPool.maxSets
		};

		Device::enableValidationLayers = m_vkSettings.device.validationLayers;

		m_device = std::make_unique<Device>(Application::instance().window(),cfg);
		m_renderer = std::make_unique<VulkanRenderer>(Application::instance().window(),*m_device);
	}

	void VulkanContext::swapBuffers() {
		VC_PROFILER_FUNCTION();
		glfwPollEvents();
	}

	void VulkanContext::waitIdle() {
		VulkanContext& i = instance();
		vkDeviceWaitIdle(i.m_device->device());
	}
}
