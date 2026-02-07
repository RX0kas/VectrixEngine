#include "VulkanDebugWidget.h"

#include "imgui.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
    VulkanDebugWidget::VulkanDebugWidget(const std::string &name) : ImGuiWidget(name) {
        m_DebugName = name;
    }

    VulkanDebugWidget::~VulkanDebugWidget() {}

    void VulkanDebugWidget::render() {
        if (!ImGui::Begin("Vulkan Debug")) {
            ImGui::End();
            return;
        }
        DebugFrameInfo frame = VulkanContext::instance().getRenderer().getCurrentFrameInfo();

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
                    (unsigned long long)buf.size,
                    (unsigned long long)buf.offset
                );
            }
        }

        ImGui::End();
    }
}
