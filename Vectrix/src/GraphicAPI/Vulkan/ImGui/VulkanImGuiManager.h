#pragma once
#include "Vectrix/ImGui/ImGuiManager.h"
#include "../Rendering/Core/Device.h"
#include "../Rendering/Core/VulkanRenderer.h"


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

private:
        VkDescriptorPool createImGuiDescriptorPool();
        static uint32_t findGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice);
	private:
		std::shared_ptr<VulkanDebugWidget> m_debugWidget;
        Device& m_device;
        Window& m_window;
        VulkanRenderer* m_renderer;
        VkDescriptorPool m_descriptorPool = nullptr;
	private:
		static VulkanImGuiManager* m_instance;
	};

}
