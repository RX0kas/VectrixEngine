#include "vcpch.h"
#include "VulkanContext.h"
#include "Vectrix/Application.h"
#include "Rendering/Mesh/MeshRegistry.h"
#include "Rendering/Mesh/VulkanVertexArray.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Mesh/Model.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"

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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		DescriptorPoolConfig cfg {64,64,64,64};
		m_device = std::make_unique<Device>(Application::instance().window(),cfg);
		m_renderer = std::make_unique<VulkanRenderer>(Application::instance().window(),*m_device);
	}

	void VulkanContext::swapBuffers() {
		VC_PROFILER_FUNCTION();
		glfwPollEvents();
	}

	void VulkanContext::registerMesh(const std::string &name, std::shared_ptr<Model> model) {
		VC_PROFILER_FUNCTION();
		auto vArrVulkan = std::dynamic_pointer_cast<VulkanVertexArray>(model->getVertexArray());
		vArrVulkan->setHandle(m_meshRegistry->registerMesh(model->getVertices(),model->getIndices()));
	}

	void VulkanContext::uploadMeshData() {
		s_instance->m_meshRegistry->uploadToGPU();
	}
}
