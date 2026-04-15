#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Vectrix/Core/Window.h"

#include <memory>
#include <vector>

#include <glm/vec4.hpp>

#include "GraphicAPI/Vulkan/Rendering/Data/DynamicSSBO.h"
#include "GraphicAPI/Vulkan/Rendering/Data/VulkanBuffer.h"
#include "GraphicAPI/Vulkan/ImGui/VulkanDebugWidget.h"
#include "Vectrix/Rendering/Mesh/Transform.h"
#include "Vectrix/Rendering/Mesh/VertexArray.h"
#include "Vectrix/Rendering/Shaders/Shader.h"

namespace Vectrix {
    struct ObjectData {
        glm::mat4 modelMatrix;
        uint32_t  textureIndex;
    };

    struct BatchInfo {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSet{};

        std::vector<std::unique_ptr<VulkanBuffer>> indirectBuffers{}; // The buffer that will send the commands, not visible in the shader

        std::vector<VkDrawIndexedIndirectCommand> commands{};
        DynamicSSBO objectDataSSBO; // The buffer that will send the objectDatas

        std::uint32_t elementCount = 0;

        BatchInfo() = delete;
        BatchInfo(BatchInfo&&) = default;
        BatchInfo& operator=(BatchInfo&&) = delete;
        BatchInfo(const BatchInfo&) = delete;
        BatchInfo& operator=(const BatchInfo&) = delete;
    };

    constexpr uint32_t MAX_OBJECTS_BATCHING = 10'000;

    class VulkanRenderer {
    public:
        VulkanRenderer(Window& window, Device & device);

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        ~VulkanRenderer();

        [[nodiscard]] VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }
        [[nodiscard]] size_t getSwapChainImageCount() const { return m_swapChain->imageCount();}
        [[nodiscard]] VkFormat getImageFormat() const { return m_swapChain->getSwapChainImageFormat(); }
        [[nodiscard]] bool isFrameInProgress() const { return m_isFrameStarted; }
        [[nodiscard]] VkImageView getSwapChainImageView(int i) const {return m_swapChain->getImageView(i);}
        [[nodiscard]] VkImage getSwapChainImage(int i) const {return m_swapChain->getSwapChainImage(i);}

        [[nodiscard]] VkFramebuffer getCurrentSwapChainFramebuffer() const { return m_swapChain->getFrameBuffer(m_swapChain->getFrameIndex()); }

        [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
            VC_CORE_ASSERT(m_isFrameStarted, "Frame not started: can't get command buffer");
            VC_CORE_ASSERT(m_currentImageIndex <= m_commandBuffers.size(), "currentImageIndex out of bounds");
            return m_commandBuffers[m_currentImageIndex];
        }

        [[nodiscard]] uint32_t getCurrentImageIndex() const {return m_currentImageIndex;}

        [[nodiscard]] int getFrameIndex() const {
            VC_CORE_ASSERT(m_isFrameStarted, "Cannot get frame index when frame not in progress");
            return m_swapChain->getFrameIndex();
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        [[nodiscard]] float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
        [[nodiscard]] VkExtent2D getSwapChainExtent() const {return m_swapChain->getSwapChainExtent();}
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

        [[nodiscard]] std::vector<VkFence> getInFlightFences() const {
            return m_swapChain->getInFlightFences();
        }

        void makeClearColor(const glm::vec4& color) {
            m_clearValue.color.float32[0] = color.r;
            m_clearValue.color.float32[1] = color.g;
            m_clearValue.color.float32[2] = color.b;
            m_clearValue.color.float32[3] = color.a;
        }

        static void submit(Shader& shader,const std::shared_ptr<VertexArray>& vertexArray,Transform transform=Transform{glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)},std::uint32_t textureIndex=0);
    private:
        friend class VulkanDebugWidget;
        friend class VulkanRendererAPI;
        friend class Renderer;
        friend class VulkanContext;
        [[nodiscard]] DebugFrameInfo getCurrentFrameInfo() const;
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();
        void cleanupSwapChain();

        void resetCache() {
            for (auto& [name, batch] : m_batchCache) {
                batch.elementCount = 0;
                batch.commands.clear();
            }
        }

        /**
         * Render all submitted data
         */
        void flush();

        Window& m_window;
        Device& m_device;
        std::shared_ptr<SwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex{ 0 };
        bool m_isFrameStarted{ false };

        VkClearValue m_clearValue = { 0.05f, 0.05f, 0.05f, 1.0f };

        Cache<std::string,BatchInfo> m_batchCache;
    };
}
