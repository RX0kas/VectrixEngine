#include "vcpch.h"
#include "VulkanContext.h"
#include "Vectrix/Application.h"

namespace Vectrix {
	VulkanContext* VulkanContext::_instance = nullptr;


	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: _WindowHandle(windowHandle)
	{
		VC_CORE_ASSERT(!_instance, "VulkanContext already exists!");
		_instance = this;
		VC_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void VulkanContext::init()
	{
		
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		_device = std::make_unique<Device>(Application::instance().window());
		_renderer = std::make_unique<Renderer>(Application::instance().window(),*_device);
	}

	void VulkanContext::swapBuffers()
	{
		glfwPollEvents();
	}

	void VulkanContext::render() {
		if (auto commandBuffer = _renderer->beginFrame()) {
			_renderer->beginSwapChainRenderPass(commandBuffer);

			for (const Shader* shader : _Shaders) {
				if (shader->_enable)
					shader->bind();
			}

			for (auto buffer : _VertexBuffer) buffer->bind();

			for (auto buffer : _IndexBuffer) buffer->bind();

			for (auto buffer : _IndexBuffer) buffer->draw();

			for (auto buffer : _VertexBuffer) buffer->draw();
			_renderer->endSwapChainRenderPass(commandBuffer);

			VulkanImGuiManager::instance().render();
			_renderer->endFrame();
		}
	}
}