#pragma once
#include "Vectrix/ImGui/ImGuiManager.h"

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

	class VulkanImGuiManager : public ImGuiManager
	{
    public:
        VulkanImGuiManager(Window& window);

        void initImGui() override;

        void render() override;
        void update() override;
        void cleanup() override;
		void attachDebugGraphicWidget() override;
		static VulkanImGuiManager& instance() { return *m_instance; }
		void destroyImGuiFramebuffers();
        void createImGuiFramebuffers();

private:
        VkDescriptorPool createImGuiDescriptorPool();
        VkRenderPass createImGuiRenderPass();
        void beginImGuiRenderPass(VkCommandBuffer commandBuffer,uint32_t imageIndex);
        void endImGuiRenderPass(VkCommandBuffer commandBuffer);
        static uint32_t findGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice);
	private:
		Ref<VulkanDebugWidget> m_debugWidget;
        Device& m_device;
        Window& m_window;
        VulkanRenderer* m_renderer;
        //VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
        VkRenderPass m_imGuiRenderPass;
        VkDescriptorPool m_descriptorPool = nullptr;
		std::vector<VkFramebuffer> m_imGuiFramebuffers;
	private:
		static VulkanImGuiManager* m_instance;
	};

}
