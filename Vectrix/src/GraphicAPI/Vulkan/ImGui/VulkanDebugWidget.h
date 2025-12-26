#ifndef VECTRIXWORKSPACE_VULKANDEBUGWIDGET_H
#define VECTRIXWORKSPACE_VULKANDEBUGWIDGET_H
#include "Vectrix/ImGui/ImGuiWidget.h"

namespace Vectrix {
    class VulkanDebugWidget : public ImGuiWidget {
    public:
        VulkanDebugWidget(const std::string& name = "VulkanDebugWidget");
        ~VulkanDebugWidget() override;

        void render() override;
    };

    // All the info needed
    struct DebugSemaphoreInfo {
        const char* name;
        bool signaled;
    };

    struct DebugFenceInfo {
        const char* name;
        bool signaled;
    };

    struct DebugPipelineInfo {
        const char* name;
        const char* vertPath;
        const char* fragPath;
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };

    struct DebugDescriptorSetInfo {
        const char* name;
        uint32_t setIndex;
        VkDescriptorSetLayout layout;
    };

    struct DebugImageInfo {
        const char* name;
        VkImageLayout layout;
        VkFormat format;
        VkExtent3D extent;
    };

    struct DebugBufferInfo {
        const char* name;
        VkDeviceSize size;
        VkDeviceSize offset;
    };

    struct DebugFrameInfo {
        uint32_t frameIndex;
        uint32_t swapchainImageIndex;

        std::vector<DebugSemaphoreInfo> semaphores;
        std::vector<DebugFenceInfo> fences;

        std::vector<DebugPipelineInfo> pipelines;
        std::vector<DebugDescriptorSetInfo> boundDescriptorSets;

        std::vector<DebugImageInfo> images;
        std::vector<DebugBufferInfo> buffers;

        uint32_t drawCalls;
        uint32_t dispatchCalls;
    };
}

#endif //VECTRIXWORKSPACE_VULKANDEBUGWIDGET_H