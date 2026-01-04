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
		p_renderer = std::make_unique<Renderer>(Application::instance().window(),*p_device);
	}

	void VulkanContext::swapBuffers()
	{
		glfwPollEvents();
	}

	void VulkanContext::render() {
		if (auto commandBuffer = p_renderer->beginFrame()) {
			p_renderer->beginSwapChainRenderPass(commandBuffer);

			for (const auto& [key, shader] : ShaderManager::instance().p_cache) {
				if (shader->_enable) {
					shader->bind();
					CameraPush::sendPush(commandBuffer,shader->_pipelineLayout);
				}
			}

			for (auto buffer : p_VertexBuffer) buffer->bind();

			for (auto buffer : p_IndexBuffer) buffer->bind();

			for (auto buffer : p_IndexBuffer) buffer->draw();

			for (auto buffer : p_VertexBuffer) buffer->draw();
			p_renderer->endSwapChainRenderPass(commandBuffer);

			VulkanImGuiManager::instance().render();
			p_renderer->endFrame();
		}
	}
}