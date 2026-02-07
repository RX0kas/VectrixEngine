#pragma once

#ifdef VC_PLATFORM_WINDOWS
#include "Platform/Windows/WinWindow.h"
#elif defined(VC_PLATFORM_LINUX)
#include "Platform/Linux/LinWindow.h"
#endif
#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "GraphicAPI/Vulkan/Rendering/VulkanRenderer.h"

#ifdef VC_PLATFORM_WINDOWS
#include "vulkan/vk_enum_string_helper.h"
#else
#include "vulkan/generated/vk_enum_string_helper.h"
#endif

namespace Vectrix {

	class VulkanImGuiManager
	{
    public:
        VulkanImGuiManager(Window& window, Device& device);

        void initImGui();

        void render();
        void update();
        void cleanup();
		static VulkanImGuiManager& instance() { return *_instance; }
		void destroyImGuiFramebuffers();
        void createImGuiFramebuffers();

private:
        VkDescriptorPool createImGuiDescriptorPool();
        VkRenderPass createImGuiRenderPass();
        void beginImGuiRenderPass(VkCommandBuffer commandBuffer,uint32_t imageIndex);
        void endImGuiRenderPass(VkCommandBuffer commandBuffer);
        static uint32_t findGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice);

        static void check_vk_result(VkResult err) {
            if (err == VK_SUCCESS)
                return;

            VC_CORE_ERROR("VkResult = {0}\n", string_VkResult(err));
        }
	private:
        Device& device;
        Window& window;
        VulkanRenderer* renderer;
        //VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
        VkRenderPass imGuiRenderPass;
        VkDescriptorPool descriptorPool = nullptr;
		std::vector<VkFramebuffer> imGuiFramebuffers;
	private:
		static VulkanImGuiManager* _instance;
	};

}
