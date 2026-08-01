#pragma once
#include "Vectrix/ImGui/ImGuiManager.h"
#include "../Rendering/Core/Device.h"
#include <string>

namespace Vectrix {
	class VulkanRenderer;

	struct DebugMemoryHeapInfo {
		const char* name;
		VkDeviceSize usedBytes;
		VkDeviceSize budgetBytes;
	};

	struct DebugFenceInfo {
		const char* name;
		bool signaled;
		bool isNull;
	};

	struct DebugPipelineInfo {
		const char* name;
		std::string vertSRC;
		std::string fragSRC;
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	struct DebugDescriptorSetInfo {
		const char* name;
		uint32_t setIndex;
		VkDescriptorSetLayout layout;
	};

	struct DebugImageInfo {
		std::string name;
		VkImageLayout layout;
		VkFormat format;
		VkExtent3D extent;
	};

	//struct DebugBufferInfo {
	//    const char* name;
	//    VkDeviceSize size;
	//    VkDeviceSize offset;
	//};

	struct DebugFrameInfo {
		uint32_t frameIndex;
		uint32_t swapchainImageIndex;

		std::vector<DebugFenceInfo> fences;

		std::vector<DebugPipelineInfo> pipelines;
		std::vector<DebugDescriptorSetInfo> boundDescriptorSets;

		std::vector<DebugImageInfo> images;
		//std::vector<DebugBufferInfo> buffers;

		uint32_t drawCalls;
		uint32_t dispatchCalls;
	};

	class VulkanImGuiManager : public ImGuiManager
	{
    public:
        VulkanImGuiManager(Window& window);

        void initImGui() override;

        void render() override;
        void update() override;
        void cleanup() override;
		void renderDebugGraphicWidget(bool& enable) override;
		static VulkanImGuiManager& instance() { return *m_instance; }

	private:
		friend class VulkanRenderer;
        VkDescriptorPool createImGuiDescriptorPool();
        static uint32_t findGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice);
        Device& m_device;
        Window& m_window;
        VulkanRenderer* m_renderer;
        VkDescriptorPool m_descriptorPool = nullptr;
		static VulkanImGuiManager* m_instance;
	};

}
