#include "vcpch.h"
#include "VulkanRenderer.h"

#include "GraphicAPI/Vulkan/ImGui/VulkanImGuiManager.h"

#ifdef VC_PLATFORM_WINDOWS
#include "Platform/Windows/WinWindow.h"
#elif defined(VC_PLATFORM_LINUX)
#include "Platform/Linux/LinWindow.h"
#else
VC_CORE_CRITICAL("The only Platform supported is Windows and Linux, VulkanRenderer can't import the platform header");
#endif

namespace Vectrix {

	Renderer::Renderer(Window& window, Device& device)
		: window{ window }, device{ device } {
		VC_CORE_INFO("Initializing Renderer");
		recreateSwapChain();

		const VkFormat f = swapChain->getSwapChainImageFormat();
		VC_CORE_ASSERT(f!=VK_FORMAT_UNDEFINED,"SwapChain image format is undefined");
		device.setImageFormat(f);
	}

	Renderer::~Renderer() {
		freeCommandBuffers();
	}

	void Renderer::recreateSwapChain() {
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
			std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
			swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*swapChain)) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}

		freeCommandBuffers();
		createCommandBuffers();
	}

	void Renderer::cleanupSwapChain() {
		swapChain->cleanup();
	}

	void Renderer::createCommandBuffers() {
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

	void Renderer::freeCommandBuffers() {
		if (commandBuffers.empty()) return;
		vkFreeCommandBuffers(
			device.device(),
			device.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer Renderer::beginFrame() {
		VC_CORE_ASSERT(!isFrameStarted, "Can't call beginFrame while already in progress");

		auto result = swapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			vkDeviceWaitIdle(device.device());
			VulkanImGuiManager::instance().destroyImGuiFramebuffers();
			recreateSwapChain();
			VulkanImGuiManager::instance().createImGuiFramebuffers();

			return VK_NULL_HANDLE;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			VC_CORE_CRITICAL("failed to acquire swap chain image!");
			VC_DEBUGBREAK();
		}

		VC_CORE_ASSERT(currentImageIndex < commandBuffers.size(), "currentImageIndex out of bounds");

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		if (commandBuffer == VK_NULL_HANDLE) {
			throw std::runtime_error("Command buffer is null!");
		}

		// Reset the buffer before reuse
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}



	void Renderer::endFrame() {
#if defined(VC_PLATFORM_WINDOWS)
		WinWindow& w = dynamic_cast<WinWindow&>(window);
#elif defined(VC_PLATFORM_LINUX)
		auto &w = dynamic_cast<LinWindow&>(window);
#else
		VC_CORE_CRITICAL("The only Platform supported is Windows and Linux, VulkanRenderer can't end the frame");
#endif

		VC_CORE_ASSERT(isFrameStarted, "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || w.wasWindowResized()) {
			w.resetWindowResizedFlag();
			vkDeviceWaitIdle(device.device());
			VulkanImGuiManager::instance().destroyImGuiFramebuffers();
			recreateSwapChain();
			VulkanImGuiManager::instance().createImGuiFramebuffers();

		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
	}

	void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
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

	void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VC_CORE_ASSERT(isFrameStarted, "Can't call endSwapChainRenderPass if frame is not in progress");
		VC_CORE_ASSERT(
			commandBuffer == getCurrentCommandBuffer(),
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

	DebugFrameInfo Renderer::getCurrentFrameInfo() {
		DebugFrameInfo f{};
		f.frameIndex = swapChain->getFrameIndex();
		f.swapchainImageIndex = currentImageIndex;

		f.semaphores = std::vector<DebugSemaphoreInfo>();
		f.fences = std::vector<DebugFenceInfo>();

		// TODO: Get the different Pipeline (shaders) and give their information

	}
}
