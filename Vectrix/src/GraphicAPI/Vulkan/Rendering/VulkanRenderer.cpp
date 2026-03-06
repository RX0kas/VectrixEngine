#include "vcpch.h"
#include "VulkanRenderer.h"

#include "Shaders/VulkanShader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "GraphicAPI/Vulkan/VulkanRendererAPI.h"
#include "GraphicAPI/Vulkan/ImGui/VulkanImGuiManager.h"
#include "Vectrix/Application.h"
#include "Vectrix/Rendering/RenderCommand.h"
#include "Vectrix/Rendering/Renderer.h"
#include "Vectrix/Rendering/Shaders/ShaderManager.h"

namespace Vectrix {

	VulkanRenderer::VulkanRenderer(Window& window, Device& device)
		: m_window{ window }, m_device{ device } {
		VC_CORE_INFO("Initializing Renderer");
		recreateSwapChain();

		const VkFormat f = m_swapChain->getSwapChainImageFormat();
		VC_CORE_ASSERT(f!=VK_FORMAT_UNDEFINED,"SwapChain image format is undefined");
		device.setImageFormat(f);


	}

	VulkanRenderer::~VulkanRenderer() {
		vkDeviceWaitIdle(m_device.device());
		freeCommandBuffers();
		cleanupSwapChain();
	}

	void VulkanRenderer::recreateSwapChain() {
		VkExtent2D extent = {m_window.getWidth(),m_window.getHeight()};
		while (extent.width == 0 || extent.height == 0) {
			extent = {m_window.getWidth(),m_window.getHeight()};
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_device.device());

		if (m_swapChain == nullptr) {
			m_swapChain = std::make_unique<SwapChain>(m_device, extent);
		}
		else {
			vkDeviceWaitIdle(m_device.device());
			VulkanImGuiManager::instance().destroyImGuiFramebuffers();
			Ref<SwapChain> oldSwapChain = std::move(m_swapChain);
			m_swapChain = std::make_unique<SwapChain>(m_device, extent, oldSwapChain);
			VulkanImGuiManager::instance().createImGuiFramebuffers();

			if (!oldSwapChain->compareSwapFormats(*m_swapChain)) {
				VC_CORE_ERROR("Swap chain image(or depth) format has changed");
			}
			oldSwapChain->cleanup();
			oldSwapChain.reset();
		}

		freeCommandBuffers();
		createCommandBuffers();
	}

	void VulkanRenderer::cleanupSwapChain() {
		m_swapChain->cleanup();
	}

	void VulkanRenderer::createCommandBuffers() {
		m_commandBuffers.resize(m_swapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			VC_CORE_TRACE("Failed to allocate command buffers");
		}
	}

	void VulkanRenderer::freeCommandBuffers() {
		if (m_commandBuffers.empty()) return;
		vkFreeCommandBuffers(m_device.device(),m_device.getCommandPool(),static_cast<uint32_t>(m_commandBuffers.size()),m_commandBuffers.data());
		m_commandBuffers.clear();
	}

	VkCommandBuffer VulkanRenderer::beginFrame() {
		VC_CORE_ASSERT(!isFrameStarted, "Frame already started");

		VkResult result = m_swapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return VK_NULL_HANDLE;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			VC_CORE_CRITICAL("failed to acquire swap chain image!");
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

		VkResult result = m_swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		bool needRecreate =
			result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			m_window.wasWindowResized();

		if (needRecreate) {
			m_window.resetWindowResizedFlag();
			isFrameStarted = false;
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		m_swapChain->advanceFrame();
	}

	void VulkanRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VC_CORE_ASSERT(isFrameStarted, "Can't call beginSwapChainRenderPass if frame is not in progress");
		VC_CORE_ASSERT(commandBuffer == getCurrentCommandBuffer(), "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapChain->getRenderPass();
		renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) const {
		VC_CORE_ASSERT(isFrameStarted, "Can't call endSwapChainRenderPass if frame is not in progress");
		VC_CORE_ASSERT(commandBuffer == getCurrentCommandBuffer(),"Can't end render pass on command buffer from a different frame");
		VC_CORE_ASSERT(commandBuffer != VK_NULL_HANDLE,"Can't end render pass if commandBuffer is VK_NULL_HANDLE");
		vkCmdEndRenderPass(commandBuffer);
	}

	void VulkanRenderer::Submit(Shader& shader, const VertexArray &vertexArray, const Transform &transform) {
		vertexArray.bind();
		RenderCommand::drawIndexed(vertexArray);
	}

	DebugFrameInfo VulkanRenderer::getCurrentFrameInfo() const {
		DebugFrameInfo f{};
		f.frameIndex = m_swapChain->getFrameIndex();
		f.swapchainImageIndex = currentImageIndex;

		f.fences = std::vector<DebugFenceInfo>();
		auto imageInFlightFences = m_swapChain->getImageInFlightFences();
		auto inFlightFences = m_swapChain->getInFlightFences();
		for (auto fe : imageInFlightFences) {
			DebugFenceInfo i{};
			i.name = "imageInFlightFences";
			if (fe==VK_NULL_HANDLE) {
				i.isNull = true;
				i.signaled = false;
			} else {
				i.signaled = vkGetFenceStatus(m_device.device(),fe)==VK_SUCCESS;
				i.isNull = false;
			}
			f.fences.push_back(i);
		}
		for (auto fe : inFlightFences) {
			DebugFenceInfo i{};
			i.name = "inFlightFences";
			i.signaled = vkGetFenceStatus(m_device.device(),fe)==VK_SUCCESS;
			f.fences.push_back(i);
		}

		std::vector<DebugPipelineInfo> pipelines;
		std::vector<DebugDescriptorSetInfo> boundDescriptorSets;

		for (auto& shader : ShaderManager::instance().getAll()) {
			auto s = std::dynamic_pointer_cast<VulkanShader>(shader);
			DebugPipelineInfo i = {s->m_name.c_str(),s->m_vertSRC,s->m_fragSRC,s->m_pipeline->getPipeline(),s->m_pipelineLayout};
			pipelines.push_back(i);
			DebugDescriptorSetInfo d{};
			d = {("SSBO-" + s->m_name).c_str(), 0, s->m_ssbo->descriptorSetLayout()};
			boundDescriptorSets.push_back(d);
		}

		f.pipelines = pipelines;
		f.boundDescriptorSets = boundDescriptorSets;
		f.images = {};
		auto defaultTexture = std::dynamic_pointer_cast<VulkanTexture>(TextureManager::getNotFoundTexture());
		f.images.push_back({"not_found",defaultTexture->getLayout(),defaultTexture->getFormat(),{defaultTexture->getWidth(),defaultTexture->getHeight(),0}});

		for (const auto&[name, tex] : TextureManager::instance().getAllWithName()) {
			auto t = std::dynamic_pointer_cast<VulkanTexture>(tex);
			DebugImageInfo i{};
			i.name = name;
			i.extent = {t->getWidth(),t->getHeight(),0};
			i.format = t->getFormat();
			i.layout = t->getLayout();
			f.images.push_back(i);
		}

		f.drawCalls = VulkanRendererAPI::getDrawCalls();
		f.dispatchCalls = 0;
		return f;
	}
}
