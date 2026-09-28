#include "vcpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "ImGui/VulkanImGuiManager.h"
#include "Rendering/Data/VulkanBuffer.h"
#include "Vectrix/Application.h"
#include "Vectrix/Debug/Profiler.h"


namespace Vectrix {
	void VulkanRendererAPI::setClearColor(const glm::vec4& color) {
		VC_PROFILER_FUNCTION();
		VulkanContext::instance().getRenderer().makeClearColor(color);
	}

	bool VulkanRendererAPI::canRender() {
		VC_PROFILER_FUNCTION();
		// beginFrame already recreates the swap chain when it returns VK_NULL_HANDLE
		return VulkanContext::instance().getRenderer().beginFrame() != VK_NULL_HANDLE;
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
}
