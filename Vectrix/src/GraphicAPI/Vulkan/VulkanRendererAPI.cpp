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

	void VulkanRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		auto i = dynamic_cast<VulkanIndexBuffer*>(vertexArray->getIndexBuffer().get());
		if (i)
			i->draw();
		for (auto& v : vertexArray->getVertexBuffers())
			dynamic_cast<VulkanVertexBuffer*>(v.get())->draw();
	}

	void VulkanRendererAPI::setupScene() {
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		auto commandBuffer = renderer.beginFrame();
		renderer.beginSwapChainRenderPass(commandBuffer);
	}

	void VulkanRendererAPI::sendScene() {
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		renderer.endSwapChainRenderPass(renderer.getCurrentCommandBuffer());
		VulkanImGuiManager::instance().render();
		renderer.endFrame();
	}
}
