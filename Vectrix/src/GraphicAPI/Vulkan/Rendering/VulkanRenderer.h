#pragma once

#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "GraphicAPI/Vulkan/Rendering/SwapChain.h"
#include "../../../Vectrix/Core/Window.h"

// std
#include <memory>
#include <vector>

#include <glm/vec4.hpp>

// Debug Widget
#include "GraphicAPI/Vulkan/ImGui/VulkanDebugWidget.h"
#include "Vectrix/Renderer/Models/Transform.h"
#include "Vectrix/Renderer/Models/VertexArray.h"
#include "Vectrix/Renderer/Shaders/Shader.h"

namespace Vectrix {
    class VulkanRenderer {
    public:
        VulkanRenderer(Window& window, Device& device);

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        ~VulkanRenderer();

        [[nodiscard]] VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }
        [[nodiscard]] size_t getSwapChainImageCount() const { return m_swapChain->imageCount();}
        [[nodiscard]] VkFormat getImageFormat() { return m_swapChain->getSwapChainImageFormat(); }
        [[nodiscard]] bool isFrameInProgress() const { return isFrameStarted; }
        [[nodiscard]] VkImageView getSwapChainImageView(int i) {return m_swapChain->getImageView(i);}

        [[nodiscard]] VkFramebuffer getCurrentSwapChainFramebuffer() { return m_swapChain->getFrameBuffer(m_swapChain->getFrameIndex()); }

        [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
            VC_CORE_ASSERT(isFrameStarted, "Frame not started: can't get command buffer");
            VC_CORE_ASSERT(currentImageIndex <= m_commandBuffers.size(), "currentImageIndex out of bounds");
            return m_commandBuffers[currentImageIndex];
        }

        [[nodiscard]] uint32_t getCurrentImageIndex() const {return currentImageIndex;}

        [[nodiscard]] int getFrameIndex() const {
            VC_CORE_ASSERT(isFrameStarted, "Cannot get frame index when frame not in progress");
            return m_swapChain->getFrameIndex();
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        [[nodiscard]] float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
        [[nodiscard]] VkExtent2D getSwapChainExtent() const {return m_swapChain->getSwapChainExtent();}
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

        void makeClearColor(const glm::vec4& color) {
            clearValue.color.float32[0] = color.r;
            clearValue.color.float32[1] = color.g;
            clearValue.color.float32[2] = color.b;
            clearValue.color.float32[3] = color.a;
        }

        static void Submit(Shader& shader,const VertexArray& vertexArray,const Transform& transform=Transform{glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)});

    private:
        friend class VulkanDebugWidget;
        friend class VulkanRendererAPI;
        DebugFrameInfo getCurrentFrameInfo() const;
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();
        void cleanupSwapChain();

        Window& m_window;
        Device& m_device;
        Ref<SwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t currentImageIndex{ 0 };
        bool isFrameStarted{ false };

        VkClearValue clearValue = { 0.05f, 0.05f, 0.05f, 1.0f };
    };
}
