#include "vcpch.h"
#include "VulkanContext.h"
#include "Vectrix/Application.h"

namespace Vectrix {
	VulkanContext* VulkanContext::s_instance = nullptr;


	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: p_WindowHandle(windowHandle)
	{
		VC_CORE_ASSERT(!s_instance, "VulkanContext already exists!");
		s_instance = this;
		VC_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void VulkanContext::init()
	{
		
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		p_device = std::make_unique<Device>(Application::instance().window());
		p_renderer = std::make_unique<VulkanRenderer>(Application::instance().window(),*p_device);
	}

	void VulkanContext::swapBuffers()
	{
		glfwPollEvents();
	}
}