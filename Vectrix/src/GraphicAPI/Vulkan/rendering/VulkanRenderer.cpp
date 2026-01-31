#include "vcpch.h"
#include "VulkanRenderer.h"

#include "VulkanShader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "GraphicAPI/Vulkan/ImGui/VulkanImGuiManager.h"
#include "Vectrix/Renderer/RenderCommand.h"

#ifdef VC_PLATFORM_WINDOWS
#include "Platform/Windows/WinWindow.h"
#elif defined(VC_PLATFORM_LINUX)
#include "Platform/Linux/LinWindow.h"
#else
VC_CORE_ERROR("The only Platform supported is Windows and Linux, VulkanRenderer can't import the platform header");
#endif

namespace Vectrix {

	VulkanRenderer::VulkanRenderer(Window& window, Device& device)
		: window{ window }, device{ device } {
		VC_CORE_INFO("Initializing Renderer");
		recreateSwapChain();

		const VkFormat f = swapChain->getSwapChainImageFormat();
		VC_CORE_ASSERT(f!=VK_FORMAT_UNDEFINED,"SwapChain image format is undefined");
		device.setImageFormat(f);
	}

	VulkanRenderer::~VulkanRenderer() {
		freeCommandBuffers();
	}

	void VulkanRenderer::recreateSwapChain() {
#if defined(VC_PLATFORM_WINDOWS)
		WinWindow& w = dynamic_cast<WinWindow&>(window);
#elif defined(VC_PLATFORM_LINUX)
		auto& w = dynamic_cast<LinWindow&>(window);
#else
		VC_CORE_CRITICAL("The only Platform supported is Windows and Linux, VulkanRenderer can't recreate the SwapChain");
#endif
		auto extent = w.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = w.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device.device());

		if (swapChain == nullptr) {
			swapChain = std::make_unique<SwapChain>(device, extent);
		}
		else {
			vkDeviceWaitIdle(device.device()); // OK ICI seulement
			VulkanImGuiManager::instance().destroyImGuiFramebuffers();
			Ref<SwapChain> oldSwapChain = std::move(swapChain);
			swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);
			VulkanImGuiManager::instance().createImGuiFramebuffers();

			if (!oldSwapChain->compareSwapFormats(*swapChain)) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
			oldSwapChain.reset();
		}

		freeCommandBuffers();
		createCommandBuffers();
	}

	void VulkanRenderer::cleanupSwapChain() {
		swapChain->cleanup();
	}

	void VulkanRenderer::createCommandBuffers() {
		commandBuffers.resize(swapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void VulkanRenderer::freeCommandBuffers() {
		if (commandBuffers.empty()) return;
		vkFreeCommandBuffers(
			device.device(),
			device.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer VulkanRenderer::beginFrame() {
		VC_CORE_ASSERT(!isFrameStarted, "Frame already started");

		VkResult result = swapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return VK_NULL_HANDLE;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}



	void VulkanRenderer::endFrame() {
		VC_CORE_ASSERT(isFrameStarted, "Frame not started");

		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
		vkEndCommandBuffer(commandBuffer);

		VkResult result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		bool needRecreate =
			result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			window.wasWindowResized();

		if (needRecreate) {
			window.resetWindowResizedFlag();
			isFrameStarted = false;
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
	}

	void VulkanRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VC_CORE_ASSERT(isFrameStarted, "Can't call beginSwapChainRenderPass if frame is not in progress");
		VC_CORE_ASSERT(
			commandBuffer == getCurrentCommandBuffer(),
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->getRenderPass();
		renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VC_CORE_ASSERT(isFrameStarted, "Can't call endSwapChainRenderPass if frame is not in progress");
		VC_CORE_ASSERT(
			commandBuffer == getCurrentCommandBuffer(),
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

	void VulkanRenderer::Submit(Shader& shader, const VertexArray &vertexArray, const Transform &transform) {
		vertexArray.bind();
		RenderCommand::drawIndexed(vertexArray);
	}

	DebugFrameInfo VulkanRenderer::getCurrentFrameInfo() {
		DebugFrameInfo f{};
		f.frameIndex = swapChain->getFrameIndex();
		f.swapchainImageIndex = currentImageIndex;

		f.semaphores = std::vector<DebugSemaphoreInfo>();
		f.fences = std::vector<DebugFenceInfo>();

		// TODO: Get the different Pipeline (shaders) and give their information

	}
}
