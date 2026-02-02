#pragma once

#include "GraphicAPI/Vulkan/rendering/Device.h"
#include "GraphicAPI/Vulkan/rendering/SwapChain.h"
#include "Vectrix/Window.h"

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

        [[nodiscard]] VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
        [[nodiscard]] size_t getSwapChainImageCount() const { return swapChain->imageCount();}
        [[nodiscard]] VkFormat getImageFormat() { return swapChain->getSwapChainImageFormat(); }
        [[nodiscard]] bool isFrameInProgress() const { return isFrameStarted; }
        [[nodiscard]] VkImageView getSwapChainImageView(int i) {return swapChain->getImageView(i);}

        [[nodiscard]] VkFramebuffer getCurrentSwapChainFramebuffer() { return swapChain->getFrameBuffer(swapChain->getFrameIndex()); }

        [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
            VC_CORE_ASSERT(isFrameStarted, "Frame not started: can't get command buffer");
            VC_CORE_ASSERT(currentImageIndex <= commandBuffers.size(), "currentImageIndex out of bounds");
            return commandBuffers[currentImageIndex];
        }

        [[nodiscard]] uint32_t getCurrentImageIndex() const {return currentImageIndex;}

        [[nodiscard]] int getFrameIndex() const {
            VC_CORE_ASSERT(isFrameStarted, "Cannot get frame index when frame not in progress");
            return swapChain->getFrameIndex();
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        [[nodiscard]] float getAspectRatio() const { return swapChain->extentAspectRatio(); }
        [[nodiscard]] VkExtent2D getSwapChainExtent() const {return swapChain->getSwapChainExtent();}
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

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

        Window& window;
        Device& device;
        Ref<SwapChain> swapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex{ 0 };
        bool isFrameStarted{ false };

        VkClearValue clearValue = { 0.01f, 0.01f, 0.01f, 1.0f };
    };
}
