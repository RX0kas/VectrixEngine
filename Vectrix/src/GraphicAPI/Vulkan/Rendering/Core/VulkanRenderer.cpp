#include "vcpch.h"
#include "VulkanRenderer.h"

#include "GraphicAPI/Vulkan/Rendering/Shaders/VulkanShader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "GraphicAPI/Vulkan/VulkanRendererAPI.h"
#include "GraphicAPI/Vulkan/ImGui/VulkanImGuiManager.h"
#include "GraphicAPI/Vulkan/Rendering/VulkanFramebuffer.h"
#include "GraphicAPI/Vulkan/Rendering/Mesh/MeshRegistry.h"
#include "GraphicAPI/Vulkan/Rendering/Mesh/VulkanVertexArray.h"
#include "GraphicAPI/Vulkan/Rendering/Shaders/Pipeline.h"
#include "Vectrix/Application.h"
#include "Vectrix/Rendering/RenderCommand.h"
#include "Vectrix/Rendering/Renderer.h"
#include "Vectrix/Rendering/Shaders/ShaderManager.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"

namespace Vectrix {

	VulkanRenderer::VulkanRenderer(Window& window, Device& device) : m_window{ window }, m_device{ device } {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Initializing Renderer");
		recreateSwapChain();

		const VkFormat f = m_swapChain->getSwapChainImageFormat();
		VC_CORE_ASSERT(f!=VK_FORMAT_UNDEFINED,"SwapChain image format is undefined");
		device.setImageFormat(f);


	}

	VulkanRenderer::~VulkanRenderer() {
		VC_PROFILER_FUNCTION();
		vkDeviceWaitIdle(m_device.device());
		freeCommandBuffers();
		cleanupSwapChain();
	}

	void VulkanRenderer::recreateSwapChain() {
		VC_PROFILER_FUNCTION();
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
			std::shared_ptr<SwapChain> oldSwapChain = std::move(m_swapChain);
			m_swapChain = std::make_unique<SwapChain>(m_device, extent, oldSwapChain);

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
		VC_PROFILER_FUNCTION();
		m_swapChain->cleanup();
	}

	void VulkanRenderer::createCommandBuffers() {
		VC_PROFILER_FUNCTION();
		m_commandBuffers.resize(m_swapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			VC_CORE_CRITICAL("Failed to allocate command buffers");
		}
	}

	void VulkanRenderer::freeCommandBuffers() {
		VC_PROFILER_FUNCTION();
		if (m_commandBuffers.empty()) return;
		vkFreeCommandBuffers(m_device.device(),m_device.getCommandPool(),static_cast<uint32_t>(m_commandBuffers.size()),m_commandBuffers.data());
		m_commandBuffers.clear();
	}

	VkCommandBuffer VulkanRenderer::beginFrame() {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!m_isFrameStarted, "Frame already started");

		VkResult result = m_swapChain->acquireNextImage(&m_currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return VK_NULL_HANDLE;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			VC_CORE_CRITICAL("failed to acquire swap chain image!");
		}

		m_isFrameStarted = true;

		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}



	void VulkanRenderer::endFrame() {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(m_isFrameStarted, "Frame not started");

		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
		vkEndCommandBuffer(commandBuffer);

		VkResult result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);

		bool needRecreate =
			result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			m_window.wasWindowResized();

		if (needRecreate) {
			m_window.resetWindowResizedFlag();
			m_isFrameStarted = false;
			return;
		}

		if (result != VK_SUCCESS) {
			VC_CORE_ERROR("Failed to present swap chain image");
		}

		m_isFrameStarted = false;
		m_swapChain->advanceFrame();
	}

	void VulkanRenderer::beginDynamicRendering(VkCommandBuffer commandBuffer) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(m_isFrameStarted, "Can't call beginSwapChainRenderPass if frame is not in progress");
		VC_CORE_ASSERT(commandBuffer == getCurrentCommandBuffer(), "Can't begin render pass on command buffer from a different frame");

		VkImageMemoryBarrier colorBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		colorBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorBarrier.image = m_swapChain->getSwapChainImage(m_currentImageIndex);
		colorBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		colorBarrier.srcAccessMask = 0;
		colorBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkImageMemoryBarrier depthBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthBarrier.image = m_swapChain->getDepthImage(m_currentImageIndex);
		depthBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
		depthBarrier.srcAccessMask = 0;
		depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkImageMemoryBarrier, 2> barriers = { colorBarrier, depthBarrier };
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, 0, nullptr, 0, nullptr,
			static_cast<uint32_t>(barriers.size()), barriers.data());

    	VkRenderingAttachmentInfoKHR colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
    	colorAttachment.imageView = m_swapChain->getImageView(m_currentImageIndex);
    	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    	colorAttachment.clearValue = m_clearValue;

    	VkRenderingAttachmentInfoKHR depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
    	depthAttachment.imageView = m_swapChain->getDepthImageView(m_currentImageIndex);
    	depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    	depthAttachment.clearValue = { .depthStencil = { 1.0f, 0 } };

    	VkRenderingInfoKHR renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
    	renderingInfo.renderArea = { {0, 0}, m_swapChain->getSwapChainExtent() };
    	renderingInfo.layerCount = 1;
    	renderingInfo.colorAttachmentCount = 1;
    	renderingInfo.pColorAttachments = &colorAttachment;
    	renderingInfo.pDepthAttachment = &depthAttachment;

    	vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

    	VkViewport viewport{ 0.0f, 0.0f,
    	    static_cast<float>(m_swapChain->getSwapChainExtent().width),
    	    static_cast<float>(m_swapChain->getSwapChainExtent().height),
    	    0.0f, 1.0f };
    	VkRect2D scissor{ {0, 0}, m_swapChain->getSwapChainExtent() };
    	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderer::endDynamicRendering(VkCommandBuffer commandBuffer) const {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(m_isFrameStarted, "Can't call endSwapChainRenderPass if frame is not in progress");
		VC_CORE_ASSERT(commandBuffer == getCurrentCommandBuffer(),"Can't end render pass on command buffer from a different frame");
		VC_CORE_ASSERT(commandBuffer != VK_NULL_HANDLE,"Can't end render pass if commandBuffer is VK_NULL_HANDLE");
		vkCmdEndRenderingKHR(commandBuffer);

		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.image = m_swapChain->getSwapChainImage(static_cast<int>(m_currentImageIndex));
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	std::vector<std::unique_ptr<VulkanBuffer>> createIndirectBuffers() {
		uint32_t frameCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		std::vector<std::unique_ptr<VulkanBuffer>> buffers;
		buffers.reserve(frameCount);

		for (uint32_t i = 0; i < frameCount; i++) {
			auto buf = std::make_unique<VulkanBuffer>(
				sizeof(VkDrawIndexedIndirectCommand), MAX_OBJECTS_BATCHING,
				VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
			buf->map();
			buffers.push_back(std::move(buf));
		}
		return buffers;
	}

	// TODO: Rework this when a material system is created
	void VulkanRenderer::submit(Shader& shader, const std::shared_ptr<VertexArray>& vertexArray, Transform transform,std::uint32_t textureIndex) {
		VC_PROFILER_FUNCTION();
		Cache<std::string, BatchInfo>& cache = VulkanContext::instance().getRenderer().m_batchCache;
		auto& vkShader = dynamic_cast<VulkanShader&>(shader);
		if (!cache.exist(vkShader.m_name)) {
			cache.emplace(
					vkShader.m_name,
					BatchInfo{
						.pipeline = vkShader.m_pipeline->getPipeline(),
						.pipelineLayout = vkShader.m_pipelineLayout,
						.descriptorSet = vkShader.m_ssbo->descriptorSet(),
						.indirectBuffers = createIndirectBuffers(),
						.commands = {},
						.objectDataSSBO = DynamicSSBO(vkShader.m_layout.get(),MAX_OBJECTS_BATCHING),
						.elementCount = 0
					});
		}
		BatchInfo& b = cache.find(vkShader.m_name)->second;
		uint32_t index = b.elementCount++;

		ObjectData currentObjectData = {
			.modelMatrix = transform.modelMatrix(),
			.textureIndex = textureIndex
		};
		uint32_t frameIndex = VulkanContext::instance().getRenderer().getFrameIndex();
		b.objectDataSSBO.write(frameIndex, index, &currentObjectData);

		MeshHandle handle = std::dynamic_pointer_cast<VulkanVertexArray>(vertexArray)->getHandle();

		VkDrawIndexedIndirectCommand command = {
			.indexCount    = handle.indexCount,
			.instanceCount = 1,
			.firstIndex    = handle.firstIndex,
			.vertexOffset  = handle.vertexOffset,
			.firstInstance = index
		};
		b.commands.push_back(command);
		b.indirectBuffers[frameIndex]->writeToBuffer(&command,sizeof(VkDrawIndexedIndirectCommand),index * sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanRenderer::flush() {
		MeshRegistry& meshRegistry = VulkanContext::instance().getMeshRegistry();
		if (meshRegistry.isEmpty()) {return;}
		VC_CORE_ASSERT(meshRegistry.isUploaded(), "MeshRegistry not uploaded! Call uploadToGPU() before rendering.");
		VC_CORE_ASSERT(meshRegistry.getVertexBuffer().getBuffer() != VK_NULL_HANDLE, "Global vertex buffer is null!");
		VC_CORE_ASSERT(meshRegistry.getIndexBuffer().getBuffer() != VK_NULL_HANDLE, "Global index buffer is null!");
		VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();

		VkExtent2D extent = m_swapChain->getSwapChainExtent();

		// If we draw to offscreen framebuffer
		auto* currentFB = dynamic_cast<VulkanFramebuffer*>(Framebuffer::getCurrentFramebuffer());
		if (currentFB!=nullptr) {
			extent = currentFB->getExtent();
		}

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height  = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = extent;
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		auto &[camera] = Renderer::getSceneData();
		uint32_t frameIndex = VulkanContext::instance().getRenderer().getFrameIndex();
		VkBuffer vertexBuf = meshRegistry.getVertexBuffer().getBuffer();

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuf, &offset);
		vkCmdBindIndexBuffer(cmd, meshRegistry.getIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		for (auto& [shaderName, batch] : m_batchCache) {
			if (batch.elementCount == 0) continue;


			std::shared_ptr<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(ShaderManager::instance().get(shaderName));

			VC_CORE_ASSERT(shader != nullptr, "Shader '{}' not found in ShaderManager", shaderName);
			VC_CORE_ASSERT(shader->m_pipelineLayout != VK_NULL_HANDLE, "PipelineLayout is null for shader '{}'", shaderName);


			VkDescriptorSet objectSet = batch.objectDataSSBO.descriptorSet(frameIndex);
			VC_CORE_ASSERT(objectSet != VK_NULL_HANDLE, "ObjectSet is null for batch '{}'", shaderName);

			if (shader->isAffectedByCamera())
				shader->sendCameraUniform(camera->getTransformationMatrix());
			shader->bind();

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->m_pipelineLayout,batch.objectDataSSBO.getSetCountID(),1, &objectSet,0, nullptr);
			batch.objectDataSSBO.flush(frameIndex);
			batch.objectDataSSBO.reset(frameIndex);

			vkCmdDrawIndexedIndirect(cmd,batch.indirectBuffers[frameIndex]->getBuffer(),0,batch.elementCount,sizeof(VkDrawIndexedIndirectCommand));
		}
	}

	DebugFrameInfo VulkanRenderer::getCurrentFrameInfo() const {
		VC_PROFILER_FUNCTION();
		DebugFrameInfo f{};
		f.frameIndex = m_swapChain->getFrameIndex();
		f.swapchainImageIndex = m_currentImageIndex;

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
