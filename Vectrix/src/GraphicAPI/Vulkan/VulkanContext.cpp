#include "vcpch.h"
#include "VulkanContext.h"
#include "Vectrix/Application.h"
#include "vk_mem_alloc.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
	VulkanContext* VulkanContext::s_instance = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!s_instance, "VulkanContext already exists!");
		s_instance = this;
		VC_CORE_ASSERT(windowHandle, "Window handle is null!");
		m_compiler = createOwn<VulkanShaderCompiler>();
	}

	VulkanContext::~VulkanContext() {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Destroying Graphic context");
		const VulkanRenderer* r = m_renderer.release();
		delete r;
		const VulkanShaderCompiler* c = m_compiler.release();
		delete c;

		const Device* d = m_device.release();
		delete d;
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
}
