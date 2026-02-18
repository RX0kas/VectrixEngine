#include "vcpch.h"
#include "VulkanImGuiManager.h"

#include "Vectrix/Application.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"



namespace Vectrix {
	VulkanImGuiManager* VulkanImGuiManager::m_instance = nullptr;

    VulkanImGuiManager::VulkanImGuiManager(Window& window) : m_device{ VulkanContext::instance().getDevice() }, m_window{ window } {
		VC_CORE_INFO("Initializing ImGuiManager");
    	VC_CORE_ASSERT(!m_instance, "ImGuiManager already exists!");
    	m_instance = this;
		m_renderer = &VulkanContext::instance().getRenderer();
	}

    void VulkanImGuiManager::attachDebugGraphicWidget() {
    	m_debugWidget = std::make_shared<VulkanDebugWidget>();
    	Application::instance().imguiLayer().addWidget(m_debugWidget);
    }

    void VulkanImGuiManager::cleanup() {
		vkDeviceWaitIdle(m_device.device());
		VC_CORE_INFO("Destroying ImGui");

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		if (m_imGuiRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(m_device.device(), m_imGuiRenderPass, nullptr);
			m_imGuiRenderPass = VK_NULL_HANDLE;
		}

		if (m_descriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(m_device.device(), m_descriptorPool, nullptr);
			m_descriptorPool = VK_NULL_HANDLE;
		}

    	for (auto f : m_imGuiFramebuffers) {
    		if (f!=VK_NULL_HANDLE)
    			vkDestroyFramebuffer(m_device.device(),f,nullptr);
    	}

		ImGui::DestroyContext();
	}

	void VulkanImGuiManager::createImGuiFramebuffers() {
    	m_imGuiFramebuffers.resize(m_renderer->getSwapChainImageCount());

    	for (size_t i = 0; i < m_imGuiFramebuffers.size(); i++) {
    		VC_CORE_ASSERT(m_renderer->getSwapChainImageView(i) != VK_NULL_HANDLE,"Swapchain image view is null for index {}", i);
    		VkImageView attachments[] = {
    			m_renderer->getSwapChainImageView(i)
			};

    		VkFramebufferCreateInfo framebufferInfo{};
    		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    		framebufferInfo.renderPass = m_imGuiRenderPass;
    		framebufferInfo.attachmentCount = 1;
    		framebufferInfo.pAttachments = attachments;
    		framebufferInfo.width = m_renderer->getSwapChainExtent().width;
    		framebufferInfo.height = m_renderer->getSwapChainExtent().height;
    		framebufferInfo.layers = 1;

    		if (vkCreateFramebuffer(m_device.device(),&framebufferInfo,nullptr,&m_imGuiFramebuffers[i]) != VK_SUCCESS)
    			throw std::runtime_error("failed to create ImGui framebuffer!");
    	}
    }

	void VulkanImGuiManager::destroyImGuiFramebuffers() {
    	for (auto fb : m_imGuiFramebuffers) {
    		if (fb != VK_NULL_HANDLE) {
    			vkDestroyFramebuffer(m_device.device(), fb, nullptr);
    		}
    	}
    	m_imGuiFramebuffers.clear();
    }



	void VulkanImGuiManager::render() {
    	VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
    	beginImGuiRenderPass(cmd,VulkanContext::instance().getRenderer().getCurrentImageIndex());
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		Application::instance().renderImGui();
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    	endImGuiRenderPass(cmd);
	}

	void VulkanImGuiManager::update() {
    }

	void VulkanImGuiManager::initImGui() {
#if defined(VC_PLATFORM_WINDOWS) || defined(VC_PLATFORM_LINUX)
		VC_CORE_INFO("initializing ImGui");

#ifdef VC_PLATFORM_WINDOWS
		auto* w = static_cast<GLFWwindow*>(dynamic_cast<WinWindow&>(m_window).getNativeWindow());
#else
		auto* w = static_cast<GLFWwindow*>(dynamic_cast<LinWindow&>(m_window).getNativeWindow());
#endif

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
#ifndef VC_PLATFORM_LINUX
    	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport
#else
    	VC_CORE_WARN("Multi-Viewport is disabled on Linux due to numerous compatibility issues.");
#endif

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForVulkan(w, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.ApiVersion = VK_API_VERSION_1_2; // same as VkApplicationInfo::apiVersion
		init_info.Instance = m_device.instance();
		init_info.PhysicalDevice = m_device.physicalDevice();
		init_info.Device = m_device.device();
		init_info.QueueFamily = findGraphicsQueueFamilyIndex(m_device.physicalDevice());
		init_info.Queue = m_device.graphicsQueue();
		//init_info.PipelineCache = g_PipelineCache;
		init_info.DescriptorPool = createImGuiDescriptorPool();
		init_info.MinImageCount = 2;
		init_info.ImageCount = m_renderer->getSwapChainImageCount(); // lets Dear ImGui know how many framebuffers and resources in general it should allocate
		init_info.Allocator = nullptr;
		init_info.PipelineInfoMain.RenderPass = createImGuiRenderPass();
		init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.CheckVkResultFn = check_vk_result; // TODO: make a Conts file for that sort of functions


		VC_CORE_ASSERT(init_info.Device != VK_NULL_HANDLE, "Vulkan device is null!");
		VC_CORE_ASSERT(init_info.PipelineInfoMain.RenderPass != VK_NULL_HANDLE, "Render pass for ImGui is null!");
		VC_CORE_ASSERT(init_info.DescriptorPool != VK_NULL_HANDLE, "Descriptor pool is null!");


		ImGui_ImplVulkan_Init(&init_info);

		//ImGui_ImplVulkan_CreateFontsTexture();

		VC_CORE_INFO("ImGui has been initialized");
    	createImGuiFramebuffers();
#else
		VC_CORE_ERROR("The only Platform supported is Windows and Linux, ImGui can't be initialized");
#endif
	}


	VkRenderPass VulkanImGuiManager::createImGuiRenderPass() {
    	VC_CORE_INFO("Creating ImGui RenderPass");
    	VkAttachmentDescription colorAttachment{};
    	colorAttachment.format = m_renderer->getImageFormat();
    	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    	colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    	VkAttachmentReference colorAttachmentRef{};
    	colorAttachmentRef.attachment = 0;
    	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    	VkSubpassDescription subpass{};
    	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    	subpass.colorAttachmentCount = 1;
    	subpass.pColorAttachments = &colorAttachmentRef;

    	VkSubpassDependency dependency{};
    	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    	dependency.dstSubpass = 0;
    	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    	dependency.srcAccessMask = 0;
    	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    	VkRenderPassCreateInfo renderPassInfo{};
    	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    	renderPassInfo.attachmentCount = 1;
    	renderPassInfo.pAttachments = &colorAttachment;
    	renderPassInfo.subpassCount = 1;
    	renderPassInfo.pSubpasses = &subpass;
    	renderPassInfo.dependencyCount = 1;
    	renderPassInfo.pDependencies = &dependency;

    	VkRenderPass renderPass = VK_NULL_HANDLE;
    	VkResult res = vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &renderPass);

    	if (res != VK_SUCCESS)
    	{
    		throw std::runtime_error("Failed to create ImGui render pass");
    	}

    	m_imGuiRenderPass = renderPass;

    	return renderPass;
    }

	void VulkanImGuiManager::beginImGuiRenderPass(VkCommandBuffer commandBuffer,uint32_t imageIndex) {
    	VC_CORE_ASSERT(m_imGuiRenderPass != VK_NULL_HANDLE, "ImGui render pass not created");
    	VC_CORE_ASSERT(imageIndex < m_imGuiFramebuffers.size(), "Invalid ImGui framebuffer index");

    	VkRenderPassBeginInfo renderPassInfo{};
    	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    	renderPassInfo.renderPass = m_imGuiRenderPass;
    	renderPassInfo.framebuffer = m_imGuiFramebuffers[imageIndex];
    	renderPassInfo.renderArea.offset = {0, 0};
    	renderPassInfo.renderArea.extent = VulkanContext::instance().getRenderer().getSwapChainExtent();

    	// Aucun clear : on dessine PAR-DESSUS la scène
    	renderPassInfo.clearValueCount = 0;
    	renderPassInfo.pClearValues = nullptr;

    	vkCmdBeginRenderPass(
			commandBuffer,
			&renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE
		);
    }

	void VulkanImGuiManager::endImGuiRenderPass(VkCommandBuffer commandBuffer) {
    	vkCmdEndRenderPass(commandBuffer);
    }

	uint32_t VulkanImGuiManager::findGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilyCount; i++) {
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				return i;  // Found graphics queue family
			}
		}

		throw std::runtime_error("No graphics queue family found!");
		return 0;
	}

	VkDescriptorPool VulkanImGuiManager::createImGuiDescriptorPool() {
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		vkCreateDescriptorPool(m_device.device(), &pool_info, nullptr, &m_descriptorPool);
		return m_descriptorPool;
	}
}
