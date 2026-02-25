#include "VulkanDebugWidget.h"
#include "vk_mem_alloc.h"
#include "Vectrix/ImGui/ImGuiWidget.h"
#include "imgui.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"
#include "Vectrix/Core/AppInfo.h"

namespace Vectrix {
    VulkanDebugWidget::VulkanDebugWidget(const std::string &name) : ImGuiWidget(name) {
        m_DebugName = name;
    }

    VulkanDebugWidget::~VulkanDebugWidget() = default;

    void VulkanDebugWidget::render() {
        if (!ImGui::Begin("Vulkan Debug")) {
            ImGui::End();
            return;
        }

        ApplicationInfo info = Application::getAppInfo();
        if (ImGui::CollapsingHeader("Application Information")) {
            ImGui::Text("Application name: %s",info.getAppName());
            ImGui::Text("Application version: %s",toString(info.getAppVersion()).c_str());
            ImGui::Text("Engine name: %s",ApplicationInfo::getEngineName());
            ImGui::Text("Engine version: %s",toString(ApplicationInfo::getEngineVersion()).c_str());
        }
        ImGui::Separator();
        DebugFrameInfo frame = VulkanContext::instance().getRenderer().getCurrentFrameInfo();
        std::vector<DebugMemoryHeapInfo> memory = collectMemoryInfo();
        ImGui::Text("Frame Index: %u", frame.frameIndex);
        ImGui::Text("Swapchain Image: %u", frame.swapchainImageIndex);
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Frame Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Draw Calls: %u", frame.drawCalls);
            ImGui::Text("Dispatch Calls: %u", frame.dispatchCalls);
        }

        if (ImGui::CollapsingHeader("Synchronization")) {
            if (ImGui::TreeNode("Semaphores")) {
                for (const auto& s : frame.semaphores) {
                    ImGui::Text("%s : %s", s.name, s.signaled ? "SIGNALED" : "WAITING");
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Fences")) {
                for (const auto& f : frame.fences) {
                    ImGui::Text("%s : %s", f.name, f.signaled ? "SIGNALED" : "UNSIGNALED");
                }
                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Pipeline")) {
            for (const auto& pipeline : frame.pipelines) {
                if (ImGui::TreeNode(pipeline.name)) {
                    // TODO: add a hot shader edition
                    // ImGui::Text("Vertex shader SRC: %s",pipeline.vertSRC.c_str());
                    // ImGui::Text("Fragment shader SRC: %s", pipeline.fragSRC.c_str());
                    ImGui::Text("Pipeline Handle: 0x%p", pipeline.pipeline);
                    ImGui::Text("Layout Handle: 0x%p", pipeline.layout);
                    ImGui::TreePop();
                }
            }
        }

        if (ImGui::CollapsingHeader("GPU Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const auto& heap : memory) {
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
                if (ImGui::TreeNode(img.name)) {
                    ImGui::Text("Format: %d", img.format);
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

        if (ImGui::CollapsingHeader("Buffers")) {
            for (const auto& buf : frame.buffers) {
                ImGui::BulletText(
                    "%s | Size: %llu | Offset: %llu",
                    buf.name,
                    static_cast<unsigned long long>(buf.size),
                    static_cast<unsigned long long>(buf.offset)
                );
            }
        }

        ImGui::End();
    }

    std::vector<DebugMemoryHeapInfo> VulkanDebugWidget::collectMemoryInfo() {
        std::vector<DebugMemoryHeapInfo> heap_infos;
        VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
        vmaGetHeapBudgets(VulkanContext::instance().getAllocator(), budgets);

        uint32_t heapCount = 0;
        VkPhysicalDeviceMemoryProperties props;
        vkGetPhysicalDeviceMemoryProperties(VulkanContext::instance().getDevice().physicalDevice(),&props);
        heapCount = props.memoryHeapCount;

        for (uint32_t i = 0; i < heapCount; ++i) {
            DebugMemoryHeapInfo info{};
            info.name = (props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) ? "DEVICE_LOCAL" : "HOST_VISIBLE";

            info.usedBytes   = budgets[i].usage;
            info.budgetBytes = budgets[i].budget;
            heap_infos.push_back(info);
        }
        return heap_infos;
    }
}
