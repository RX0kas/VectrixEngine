#include "vcpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "ImGui/VulkanImGuiManager.h"


namespace Vectrix {

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		VulkanContext::instance().getRenderer().makeClearColor(color);
	}

	void VulkanRendererAPI::Clear()
	{
		VC_CORE_WARN("VulkanRendererAPI::Clear() is not implemented");
	}

	void VulkanRendererAPI::DrawIndexed(const VertexArray& vertexArray)
	{
		if (vertexArray.getIndexBuffer()) {
			// Si nous avons un index buffer, utilisez-le
			auto i = dynamic_cast<VulkanIndexBuffer*>(vertexArray.getIndexBuffer().get());
			if (i) {
				i->draw();
			}
		} else {
			// Sinon, dessinez chaque vertex buffer sans index
			for (auto& v : vertexArray.getVertexBuffers()) {
				dynamic_cast<VulkanVertexBuffer*>(v.get())->draw();
			}
		}
		s_drawCalls++;
	}

	bool VulkanRendererAPI::setupScene() {
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		auto commandBuffer = renderer.beginFrame();
		s_drawCalls = 0;

		if (commandBuffer == VK_NULL_HANDLE) {
			renderer.recreateSwapChain();
			return false;
		}

		renderer.beginSwapChainRenderPass(commandBuffer);
		return true;
	}

	void VulkanRendererAPI::sendScene() {
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		renderer.endSwapChainRenderPass(renderer.getCurrentCommandBuffer());
		VulkanImGuiManager::instance().render();
		renderer.endFrame();
	}
}
