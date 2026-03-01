#include "vcpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "ImGui/VulkanImGuiManager.h"
#include "Rendering/VulkanBuffer.h"
#include "Vectrix/Application.h"
#include "Vectrix/ImGui/ImGuiLayer.h"


namespace Vectrix {
	uint32_t VulkanRendererAPI::s_drawCalls = 0;

	void VulkanRendererAPI::setClearColor(const glm::vec4& color)
	{
		VulkanContext::instance().getRenderer().makeClearColor(color);
	}

	bool VulkanRendererAPI::prepareFrame() {
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		auto commandBuffer = renderer.beginFrame();
		s_drawCalls = 0;

		if (commandBuffer == VK_NULL_HANDLE) {
			renderer.recreateSwapChain();
			VC_CORE_INFO("commandBuffer is NULL");
			return false;
		}

		renderer.beginSwapChainRenderPass(commandBuffer);
		return true;
	}

	void VulkanRendererAPI::endFrame() {
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		renderer.endSwapChainRenderPass(renderer.getCurrentCommandBuffer());
	}

	void VulkanRendererAPI::sendFrame() {
		VulkanContext::instance().getRenderer().endFrame();
	}

	void VulkanRendererAPI::drawIndexed(const VertexArray& vertexArray)
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
}
