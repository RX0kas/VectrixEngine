#include "vcpch.h"
#include "VulkanImGuiManager.h"

#include "Vectrix/Application.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"



namespace Vectrix {
	VulkanImGuiManager* VulkanImGuiManager::m_instance = nullptr;

    VulkanImGuiManager::VulkanImGuiManager(Window& window) : m_device{ VulkanContext::instance().getDevice() }, m_window{ window } {
		VC_CORE_INFO("Initializing ImGuiManager");
    	VC_CORE_ASSERT(!m_instance, "ImGuiManager already exists");
    	m_instance = this;
		m_renderer = &VulkanContext::instance().getRenderer();
	}

	std::vector<DebugMemoryHeapInfo> collectMemoryInfo(VmaAllocator allocator) {
    	std::vector<DebugMemoryHeapInfo> heap_infos;
    	VmaBudget budgets[VK_MAX_MEMORY_HEAPS];

    	vmaGetHeapBudgets(allocator, budgets);

    	uint32_t heapCount = 0;
    	VkPhysicalDeviceMemoryProperties props;
    	vkGetPhysicalDeviceMemoryProperties(VulkanContext::instance().getDevice().physicalDevice(),&props);
    	heapCount = props.memoryHeapCount;

    	for (uint32_t i = 0; i < heapCount; ++i) {
    		DebugMemoryHeapInfo info{};
    		info.name = (props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) ? "DEVICE_LOCAL" : "HOST_VISIBLE";

    		info.usedBytes = budgets[i].usage;
    		info.budgetBytes = budgets[i].budget;
    		heap_infos.push_back(info);
    	}
    	return heap_infos;
    }

    void VulkanImGuiManager::renderDebugGraphicWidget(bool& enable) {
    	if (!ImGui::Begin("Vulkan Debug", &enable)) {
            ImGui::End();
            return;
        }

        const ApplicationInfo info = Application::getAppInfo();
        if (ImGui::CollapsingHeader("Application Information")) {
            ImGui::Text("Application name: %s",info.getAppName());
            ImGui::Text("Application version: %s",toString(info.getAppVersion()).c_str());
            ImGui::Text("Engine name: %s",ApplicationInfo::getEngineName());
            ImGui::Text("Engine version: %s",toString(ApplicationInfo::getEngineVersion()).c_str());
            ImGui::Text("FPS: %f",1/Application::instance().getDeltaTime().getSeconds());
        }
        ImGui::Separator();
        const DebugFrameInfo frame = VulkanContext::instance().getRenderer().getCurrentFrameInfo();
        const std::vector<DebugMemoryHeapInfo> memorySSBO = collectMemoryInfo(VulkanContext::instance().getSSBOAllocator());
        const std::vector<DebugMemoryHeapInfo> memoryBuffer = collectMemoryInfo(VulkanContext::instance().getBufferAllocator());
        const std::vector<DebugMemoryHeapInfo> memoryTexture = collectMemoryInfo(VulkanContext::instance().getTextureAllocator());
        ImGui::Text("Frame Index: %u", frame.frameIndex);
        ImGui::Text("Swapchain Image: %u", frame.swapchainImageIndex);
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Frame Stats")) {
            ImGui::Text("Draw Calls: %u", frame.drawCalls);
            ImGui::Text("Dispatch Calls: %u", frame.dispatchCalls);
        }

        if (ImGui::CollapsingHeader("Synchronization")) {
            if (ImGui::TreeNode("Fences")) {
                for (const auto& f : frame.fences) {
                    ImGui::Text("%s : %s", f.name, f.isNull ? "VK_NULL_HANDLE" : f.signaled ? "SIGNALED" : "UNSIGNALED");
                }
                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Pipeline")) {
            for (const auto& pipeline : frame.pipelines) {
                char name[256] = "Pipeline - ";
                strcat(name,pipeline.name);
                if (ImGui::TreeNode(name)) {
                    // TODO: add a hot shader edition
                    // ImGui::Text("Vertex shader SRC: %s",pipeline.vertSRC.c_str());
                    // ImGui::Text("Fragment shader SRC: %s", pipeline.fragSRC.c_str());
                    ImGui::Text("Pipeline Handle: 0x%p", pipeline.pipeline);
                    ImGui::Text("Layout Handle: 0x%p", pipeline.layout);
                    ImGui::TreePop();
                }
            }
        }

        if (ImGui::CollapsingHeader("GPU Memory")) {
            if (ImGui::TreeNode("SSBOMem")) {
                for (const auto& heap : memorySSBO) {
                    float fraction = 0.0f;
                    if (heap.budgetBytes > 0) {
                        fraction = static_cast<float>(heap.usedBytes) / static_cast<float>(heap.budgetBytes);
                    }

                    ImGui::Text("%s", heap.name);

                    ImVec4 color;
                    if (fraction < 0.6f)
                        color = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // green
                    else if (fraction < 0.85f)
                        color = ImVec4(0.9f, 0.7f, 0.2f, 1.0f); // orange
                    else
                        color = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // red

                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
                    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f));
                    ImGui::PopStyleColor();


                    ImGui::Text("Used: %.2f MB / %.2f MB",static_cast<float>(heap.usedBytes) / (1024.0f * 1024.0f),static_cast<float>(heap.budgetBytes) / (1024.0f * 1024.0f));

                    ImGui::Separator();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("ImagesMem")) {
                for (const auto& heap : memoryTexture) {
                    float fraction = 0.0f;
                    if (heap.budgetBytes > 0) {
                        fraction = static_cast<float>(heap.usedBytes) / static_cast<float>(heap.budgetBytes);
                    }

                    ImGui::Text("%s", heap.name);

                    ImVec4 color;
                    if (fraction < 0.6f)
                        color = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // green
                    else if (fraction < 0.85f)
                        color = ImVec4(0.9f, 0.7f, 0.2f, 1.0f); // orange
                    else
                        color = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // red

                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
                    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f));
                    ImGui::PopStyleColor();


                    ImGui::Text("Used: %.2f MB / %.2f MB",static_cast<float>(heap.usedBytes) / (1024.0f * 1024.0f),static_cast<float>(heap.budgetBytes) / (1024.0f * 1024.0f));

                    ImGui::Separator();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("BuffersMem")) {
                for (const auto& heap : memoryBuffer) {
                    float fraction = 0.0f;
                    if (heap.budgetBytes > 0) {
                        fraction = static_cast<float>(heap.usedBytes) / static_cast<float>(heap.budgetBytes);
                    }

                    ImGui::Text("%s", heap.name);

                    ImVec4 color;
                    if (fraction < 0.6f)
                        color = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // green
                    else if (fraction < 0.85f)
                        color = ImVec4(0.9f, 0.7f, 0.2f, 1.0f); // orange
                    else
                        color = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // red

                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
                    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f));
                    ImGui::PopStyleColor();


                    ImGui::Text("Used: %.2f MB / %.2f MB",static_cast<float>(heap.usedBytes) / (1024.0f * 1024.0f),static_cast<float>(heap.budgetBytes) / (1024.0f * 1024.0f));

                    ImGui::Separator();
                }
                ImGui::TreePop();
            }
        }


        if (ImGui::CollapsingHeader("Descriptor Sets")) {
            for (const auto& set : frame.boundDescriptorSets) {
                ImGui::BulletText(
                    "Set %u (%s) | Layout: 0x%p",
                    set.setIndex,
                    set.name,
                    set.layout
                );
            }
        }

        if (ImGui::CollapsingHeader("Images")) {
            for (const auto& img : frame.images) {
                char name[256] = "Image - ";
                strcat(name,img.name.c_str());
                if (ImGui::TreeNode(name)) {
                    ImGui::Text("Format: %s", string_VkFormat(img.format));
                    ImGui::Text("Layout: %d", img.layout);
                    ImGui::Text(
                        "Extent: %u x %u x %u",
                        img.extent.width,
                        img.extent.height,
                        img.extent.depth
                    );
                    ImGui::TreePop();
                }
            }
        }

        ImGui::End();
    }

    void VulkanImGuiManager::cleanup() {
		vkDeviceWaitIdle(m_device.device());
		VC_CORE_INFO("Destroying ImGui");

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		if (m_descriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(m_device.device(), m_descriptorPool, nullptr);
			m_descriptorPool = VK_NULL_HANDLE;
		}
	}



	void VulkanImGuiManager::render() {
    	VkCommandBuffer cmd = VulkanContext::instance().getRenderer().getCurrentCommandBuffer();
		VkExtent2D extent = VulkanContext::instance().getRenderer().getSwapChainExtent();
		uint32_t imageIndex = VulkanContext::instance().getRenderer().getCurrentImageIndex();
		VkImageView imageView = VulkanContext::instance().getRenderer().getSwapChainImageView(static_cast<int>(imageIndex));

		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.image = VulkanContext::instance().getRenderer().getSwapChainImage(imageIndex);
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkRenderingAttachmentInfoKHR colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
		colorAttachment.imageView = imageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfoKHR renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
		renderingInfo.renderArea = { {0, 0}, extent };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;

		vkCmdBeginRenderingKHR(cmd, &renderingInfo);

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    	ImGuizmo::BeginFrame();
		Application::instance().renderImGui();
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

		vkCmdEndRenderingKHR(cmd);

		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;
		vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

	void VulkanImGuiManager::update() {}

	void VulkanImGuiManager::initImGui() {
		VC_CORE_INFO("Initializing ImGui");
    	VC_CORE_ASSERT(vkGetInstanceProcAddr != nullptr, "Volk global functions not loaded!");
    	VC_CORE_ASSERT(vkGetDeviceProcAddr != nullptr, "Volk device proc addr function is null!");


		auto* w = static_cast<GLFWwindow*>(m_window.getNativeWindow());

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
    	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		#ifndef VC_PLATFORM_LINUX
		    	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		    	io.ConfigDpiScaleFonts    = true;
		    	io.ConfigDpiScaleViewports = true;
		#else
		    	VC_CORE_WARN("Multi-Viewport and DPI scaling disabled on Linux due to compatibility issues");
		    	io.ConfigDpiScaleFonts    = false;
		    	io.ConfigDpiScaleViewports = false;
		#endif

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;

		ImGui_ImplGlfw_InitForVulkan(w, true);
		#ifdef VC_PLATFORM_LINUX
		    	static GLFWmonitorfun s_imguiMonitorCallback = glfwSetMonitorCallback(nullptr);

		    	glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) {
					if (s_imguiMonitorCallback && ImGui::GetCurrentContext() != nullptr) {
						s_imguiMonitorCallback(monitor, event);
					}
				});
		#endif

    	ImGui_ImplVulkan_InitInfo init_info = {};
    	init_info.ApiVersion = VK_API_VERSION_1_3;
    	init_info.Instance = m_device.instance();
    	init_info.PhysicalDevice = m_device.physicalDevice();
    	init_info.Device = m_device.device();
    	init_info.QueueFamily = findGraphicsQueueFamilyIndex(m_device.physicalDevice());
    	init_info.Queue = m_device.graphicsQueue();
    	init_info.DescriptorPool = createImGuiDescriptorPool();
    	init_info.MinImageCount = 2;
    	init_info.ImageCount = m_renderer->getSwapChainImageCount();
    	init_info.Allocator = nullptr;
    	init_info.CheckVkResultFn = VulkanContext::check_vk_result;

    	init_info.UseDynamicRendering = true;
    	init_info.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
    	init_info.PipelineInfoMain.Subpass = 0;
    	init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
    	pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    	pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    	VkFormat colorFormat = m_renderer->getImageFormat();
    	pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;

    	init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;

#ifndef VC_PLATFORM_LINUX
    	init_info.PipelineInfoMain.SwapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
#endif

		VC_CORE_ASSERT(init_info.Device != VK_NULL_HANDLE, "Vulkan device is null!");
		VC_CORE_ASSERT(init_info.DescriptorPool != VK_NULL_HANDLE, "Descriptor pool is null!");
    	PFN_vkVoidFunction testFunc = vkGetDeviceProcAddr(init_info.Device, "vkCmdBeginRenderingKHR");
    	VC_CORE_ASSERT(testFunc != nullptr, "vkCmdBeginRenderingKHR not found – dynamic rendering feature not enabled?");



		ImGui_ImplVulkan_Init(&init_info);

		//ImGui_ImplVulkan_CreateFontsTexture();

    	//createImGuiFramebuffers();
		VC_CORE_INFO("ImGui has been initialized");
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

		VC_CORE_ERROR("No graphics queue family found");
	}

	VkDescriptorPool VulkanImGuiManager::createImGuiDescriptorPool() {
		const VkDescriptorPoolSize pool_sizes[] = {
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
		pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;

		vkCreateDescriptorPool(m_device.device(), &pool_info, nullptr, &m_descriptorPool);
		return m_descriptorPool;
	}
}
