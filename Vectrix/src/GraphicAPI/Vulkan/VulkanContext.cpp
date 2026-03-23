#include "vcpch.h"
#include "VulkanContext.h"
#include "Vectrix/Application.h"
#include "Rendering/Mesh/MeshRegistry.h"
#include "Rendering/Mesh/VulkanVertexArray.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Mesh/Model.h"

namespace Vectrix {
	VulkanContext* VulkanContext::s_instance = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!s_instance, "VulkanContext already exists!");
		s_instance = this;
		VC_CORE_ASSERT(windowHandle, "Window handle is null!");
		m_compiler = createOwn<VulkanShaderCompiler>();
		m_meshRegistry = createOwn<MeshRegistry>();
	}

	VulkanContext::~VulkanContext() {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Destroying Graphic context");

		VC_DELETE_OWN(m_renderer);
		VC_DELETE_OWN(m_compiler);
		VC_DELETE_OWN(m_meshRegistry);
		VC_DELETE_OWN(m_device);
	}

	void VulkanContext::init() {
		VC_PROFILER_FUNCTION();
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		DescriptorPoolConfig cfg {64,64,64,64};
		m_device = createOwn<Device>(Application::instance().window(),cfg);
		m_renderer = createOwn<VulkanRenderer>(Application::instance().window(),*m_device);
	}

	void VulkanContext::swapBuffers() {
		VC_PROFILER_FUNCTION();
		glfwPollEvents();
	}

	void VulkanContext::registerMesh(const std::string &name, Ref<Model> model) {
		VC_PROFILER_FUNCTION();
		auto vArrVulkan = std::dynamic_pointer_cast<VulkanVertexArray>(model->getVertexArray());
		vArrVulkan->setHandle(m_meshRegistry->registerMesh(model->getVertices(),model->getIndices()));
	}

	void VulkanContext::uploadMeshData() {
		s_instance->m_meshRegistry->uploadToGPU();
	}
}
