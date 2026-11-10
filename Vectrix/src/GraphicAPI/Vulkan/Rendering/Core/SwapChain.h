#pragma once
#include "Vectrix/Core/Log.h"

#include <optional>

#include "Device.h"

namespace Vectrix {
	class SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        /// @param vsync Overrides the present mode of the settings: true for fifo, false for mailbox (or immediate)
        SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::optional<bool> vsync = std::nullopt);
        SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous, std::optional<bool> vsync = std::nullopt);

        ~SwapChain();

        void cleanup();

        SwapChain(const SwapChain&) = delete;
        void operator=(const SwapChain&) = delete;

        [[nodiscard]] VkImageView getImageView(uint32_t index) const { return m_swapChainImageViews[index]; }
        [[nodiscard]] VkImageView getDepthImageView(uint32_t index) const { return m_depthImageViews[index]; }
        [[nodiscard]] size_t imageCount() const { return m_swapChainImages.size(); }
	    [[nodiscard]] VkImage getSwapChainImage(uint32_t index) const { return m_swapChainImages[index]; }
	    [[nodiscard]] VkImage getDepthImage(uint32_t index) const { return m_depthImages[index]; }
        [[nodiscard]] VkFormat getSwapChainImageFormat() const { return m_swapChainImageFormat; }
        [[nodiscard]] VkExtent2D getSwapChainExtent() const { return m_swapChainExtent; }
        /// The present mode the swap chain was created with (what the surface allowed of the one asked for)
        [[nodiscard]] VkPresentModeKHR getPresentMode() const { return m_presentMode; }

        [[nodiscard]] float extentAspectRatio() const {
            return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
        }
        [[nodiscard]] VkFormat findDepthFormat() const;

        VkResult acquireNextImage(uint32_t* imageIndex) const;
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex);

		[[nodiscard]] bool compareSwapFormats(const SwapChain& swapChain) const {
			return swapChain.m_swapChainDepthFormat == m_swapChainDepthFormat &&
				swapChain.m_swapChainImageFormat == m_swapChainImageFormat;
		}

	    [[nodiscard]] int getFrameIndex() const {return static_cast<int>(m_currentFrame);}

	    void advanceFrame() {
            m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        std::vector<VkFence> getInFlightFences() {
            return m_inFlightFences;
        }
    private:
		void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();

        void createSyncObjects();

        // Helper functions
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR preferred) const;
        /// The present mode for an explicit V-Sync request (setVSync), else the settings' one
        VkPresentModeKHR chooseVSyncPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync) const;
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        VkFormat m_swapChainImageFormat;
        VkFormat m_swapChainDepthFormat;
        VkExtent2D m_swapChainExtent{};


        std::vector<VkImage> m_depthImages;
        std::vector<VmaAllocation> m_depthImageAllocations;
        std::vector<VkImageView> m_depthImageViews;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        Device& m_device;
        VkExtent2D m_windowExtent;

        VkSwapchainKHR m_swapChain{};
		std::shared_ptr<SwapChain> m_oldSwapChain;
        std::optional<bool> m_vsync;
        VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;

        std::vector<VkSemaphore> m_imageAvailableSemaphores; // par frame
        std::vector<VkSemaphore> m_renderFinishedSemaphores; // par image
        std::vector<VkFence> m_inFlightFences; // par frame

        size_t m_currentFrame = 0;
    };
}