#include "vcpch.h"
#include "VulkanContext.h"
#include "Vectrix/Application.h"
#include "vk_mem_alloc.h"

namespace Vectrix {
	VulkanContext* VulkanContext::s_instance = nullptr;


	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		VC_CORE_ASSERT(!s_instance, "VulkanContext already exists!");
		s_instance = this;
		VC_CORE_ASSERT(windowHandle, "Window handle is null!");
		m_compiler = std::make_unique<VulkanShaderCompiler>();
	}

	VulkanContext::~VulkanContext() {
		VC_CORE_INFO("Destroying Graphic context");
		VulkanRenderer* r = m_renderer.release();
		delete r;
		VulkanShaderCompiler* c = m_compiler.release();
		delete c;

		Device* d = m_device.release();
		delete d;
	}

	void VulkanContext::init()
	{
		
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		DescriptorPoolConfig cfg {64,64,64,64};
		m_device = std::make_unique<Device>(Application::instance().window(),cfg);
		m_renderer = std::make_unique<VulkanRenderer>(Application::instance().window(),*m_device);
	}

	void VulkanContext::swapBuffers()
	{
		glfwPollEvents();
	}
}