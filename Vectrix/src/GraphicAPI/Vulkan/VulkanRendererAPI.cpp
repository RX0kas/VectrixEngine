#include "vcpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "ImGui/VulkanImGuiManager.h"
#include "Rendering/Data/VulkanBuffer.h"
#include "Vectrix/Application.h"
#include "Vectrix/Debug/Profiler.h"


namespace Vectrix {
	uint32_t VulkanRendererAPI::s_drawCalls = 0;

	void VulkanRendererAPI::setClearColor(const glm::vec4& color) {
		VC_PROFILER_FUNCTION();
		VulkanContext::instance().getRenderer().makeClearColor(color);
	}

	bool VulkanRendererAPI::canRender() {
		VC_PROFILER_FUNCTION();
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		auto commandBuffer = renderer.beginFrame();

		if (commandBuffer == VK_NULL_HANDLE) {
			renderer.recreateSwapChain();
			return false;
		}

		return true;
	}

	void VulkanRendererAPI::beginFrame() {
		VC_PROFILER_FUNCTION();
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		renderer.beginDynamicRendering(renderer.getCurrentCommandBuffer());
	}

	void VulkanRendererAPI::endFrame() {
		VC_PROFILER_FUNCTION();
		VulkanRenderer& renderer = VulkanContext::instance().getRenderer();
		renderer.endDynamicRendering(renderer.getCurrentCommandBuffer());
	}

	void VulkanRendererAPI::sendFrame() {
		VC_PROFILER_FUNCTION();
		VulkanContext::instance().getRenderer().endFrame();
	}

	void VulkanRendererAPI::drawIndexed(const VertexArray& vertexArray)	{
		VC_PROFILER_FUNCTION();
		if (vertexArray.getIndexBuffer()) {
			auto i = dynamic_cast<VulkanIndexBuffer*>(vertexArray.getIndexBuffer().get());
			if (i) {
				i->draw();
			}
		} else {
			for (auto& v : vertexArray.getVertexBuffers()) {
				dynamic_cast<VulkanVertexBuffer*>(v.get())->draw();
			}
		}
		s_drawCalls++;
	}
}
