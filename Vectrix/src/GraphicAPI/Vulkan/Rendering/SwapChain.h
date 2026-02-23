#pragma once
#include "SSBO.h"
#include "vcpch.h"

#include "GraphicAPI/Vulkan/Rendering/Device.h"

namespace Vectrix {
	class SwapChain {

    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(Device& deviceRef, VkExtent2D windowExtent);
        SwapChain(Device& deviceRef, VkExtent2D windowExtent, Ref<SwapChain> previous);

        ~SwapChain();

        void cleanup();

        SwapChain(const SwapChain&) = delete;
        void operator=(const SwapChain&) = delete;

        [[nodiscard]] VkFramebuffer getFrameBuffer(int index) const { return m_swapChainFramebuffers[index]; }
        void destroyFrameBuffers() {
            VC_CORE_INFO("Destroying FrameBuffers");
            if (!m_swapChainFramebuffers.empty()) {
                for (auto framebuffer : m_swapChainFramebuffers) {
                    if (framebuffer != VK_NULL_HANDLE) {
                        vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);
                    }
                }
                m_swapChainFramebuffers.clear();
            }

        }

        [[nodiscard]] VkSwapchainKHR swapChain() const {return m_swapChain;}


        [[nodiscard]] VkRenderPass getRenderPass() const { return m_renderPass; }
        [[nodiscard]] VkImageView getImageView(int index) const { return m_swapChainImageViews[index]; }
        std::vector<VkImageView> getImageViews() { return m_swapChainImageViews; }
        [[nodiscard]] size_t imageCount() const { return m_swapChainImages.size(); }
        [[nodiscard]] VkFormat getSwapChainImageFormat() const { return m_swapChainImageFormat; }
        [[nodiscard]] VkExtent2D getSwapChainExtent() const { return m_swapChainExtent; }
        [[nodiscard]] uint32_t width() const { return m_swapChainExtent.width; }
        [[nodiscard]] uint32_t height() const { return m_swapChainExtent.height; }

        [[nodiscard]] float extentAspectRatio() const {
            return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
        }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex);

		[[nodiscard]] bool compareSwapFormats(const SwapChain& swapChain) const {
			return swapChain.m_swapChainDepthFormat == m_swapChainDepthFormat &&
				swapChain.m_swapChainImageFormat == m_swapChainImageFormat;
		}

        void createFramebuffers();

        void destroyViews() {
            destroyImageviews();
            destroyDepthImageviews();
            destroySwapChainImages();
            destroyDepthImages();
        }

        void destroyRenderPass() {
            if (m_renderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);
                m_renderPass = VK_NULL_HANDLE;
            }
        }
        void destroySynchronisationObject() {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                if (m_renderFinishedSemaphores[i]!=VK_NULL_HANDLE) {
                    vkDestroySemaphore(m_device.device(), m_renderFinishedSemaphores[i], nullptr);
                    m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
                }
                if (m_imageAvailableSemaphores[i]!=VK_NULL_HANDLE) {
                    vkDestroySemaphore(m_device.device(), m_imageAvailableSemaphores[i], nullptr);
                    m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
                }
                if (m_inFlightFences[i]!=VK_NULL_HANDLE) {
                    vkDestroyFence(m_device.device(), m_inFlightFences[i], nullptr);
                    m_inFlightFences[i] = VK_NULL_HANDLE;
                }
            }
        }

	    [[nodiscard]] int getFrameIndex() const {return static_cast<int>(m_currentFrame);}

	    void advanceFrame() {
            m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        std::vector<VkFence> getInFlightFences() {
            return m_inFlightFences;
        }
    private:
        void destroyImageviews() {
            if (!m_swapChainImageViews.empty()) {
                for (auto view  : m_swapChainImageViews) {
                    if (view  != VK_NULL_HANDLE) {
                        vkDestroyImageView(m_device.device(), view, nullptr);
                    }
                }
                m_swapChainImageViews.clear();
            }
        }

        void destroyDepthImageviews() {
            if (!m_depthImageViews.empty()) {
                for (auto view  : m_depthImageViews) {
                    if (view  != VK_NULL_HANDLE) {
                        vkDestroyImageView(m_device.device(), view, nullptr);
                    }
                }
                m_depthImageViews.clear();
            }
        }

        void destroySwapChainImages() {
            if (!m_swapChainImages.empty()) {
                for (auto image  : m_swapChainImages) {
                    if (image  != VK_NULL_HANDLE) {
                        vkDestroyImage(m_device.device(), image, nullptr);
                    }
                }
                m_depthImageViews.clear();
            }
        }

	    void destroyDepthImages() {
            for (size_t i = 0; i < m_depthImages.size(); i++) {
                m_device.destroyImage(m_depthImages[i], m_depthImageAllocations[i]);
            }
            m_depthImages.clear();
            m_depthImageAllocations.clear();
        }

		void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();

        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat m_swapChainImageFormat;
        VkFormat m_swapChainDepthFormat;
        VkExtent2D m_swapChainExtent;

        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;

        std::vector<VkImage> m_depthImages;
        std::vector<VmaAllocation> m_depthImageAllocations;
        std::vector<VkImageView> m_depthImageViews;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        Device& m_device;
        VkExtent2D m_windowExtent;

        VkSwapchainKHR m_swapChain;
		Ref<SwapChain> m_oldSwapChain;

        std::vector<VkSemaphore> m_imageAvailableSemaphores; // par frame
        std::vector<VkSemaphore> m_renderFinishedSemaphores; // par image
        std::vector<VkFence> m_inFlightFences; // par frame
        std::vector<VkFence> m_imagesInFlight; // par image

        size_t m_currentFrame = 0;
    };
}