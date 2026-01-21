#pragma once
#include "SSBO.h"
#include "vcpch.h"

#include "GraphicAPI/Vulkan/rendering/Device.h"

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

        [[nodiscard]] VkFramebuffer getFrameBuffer(int index) const { return swapChainFramebuffers[index]; }
        void destroyFrameBuffers() {
            if (!swapChainFramebuffers.empty()) {
                for (auto framebuffer : swapChainFramebuffers) {
                    if (framebuffer != VK_NULL_HANDLE) {
                        vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
                    }
                }
                swapChainFramebuffers.clear();
            }

        }

        [[nodiscard]] VkSwapchainKHR swapChain() const {return _swapChain;}


        [[nodiscard]] VkRenderPass getRenderPass() const { return renderPass; }
        [[nodiscard]] VkImageView getImageView(int index) const { return swapChainImageViews[index]; }
        std::vector<VkImageView> getImageViews() { return swapChainImageViews; }
        [[nodiscard]] size_t imageCount() const { return swapChainImages.size(); }
        [[nodiscard]] VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
        [[nodiscard]] VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
        [[nodiscard]] uint32_t width() const { return swapChainExtent.width; }
        [[nodiscard]] uint32_t height() const { return swapChainExtent.height; }

        [[nodiscard]] float extentAspectRatio() const {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		[[nodiscard]] bool compareSwapFormats(const SwapChain& swapChain) const {
			return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
				swapChain.swapChainImageFormat == swapChainImageFormat;
		}

        void createFramebuffers();

        void destroyViews() {
            destroyImageviews();
            destroyDepthImageviews();
            destroySwapChainImages();
            destroyDepthImages();
            destroyDepthMemorys();
        }

        void destroyRenderPass() {
            if (renderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(device.device(), renderPass, nullptr);
                renderPass = VK_NULL_HANDLE;
            }
        }
        void destroySynchronisationObject() {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                if (renderFinishedSemaphores[i]!=VK_NULL_HANDLE) {
                    vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
                    renderFinishedSemaphores[i] = VK_NULL_HANDLE;
                }
                if (imageAvailableSemaphores[i]!=VK_NULL_HANDLE) {
                    vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
                    imageAvailableSemaphores[i] = VK_NULL_HANDLE;
                }
                if (inFlightFences[i]!=VK_NULL_HANDLE) {
                    vkDestroyFence(device.device(), inFlightFences[i], nullptr);
                    inFlightFences[i] = VK_NULL_HANDLE;
                }
            }
        }

	    [[nodiscard]] int getFrameIndex() const {return static_cast<int>(currentFrame);}
    private:
        void destroyImageviews() {
            if (!swapChainImageViews.empty()) {
                for (auto view  : swapChainImageViews) {
                    if (view  != VK_NULL_HANDLE) {
                        vkDestroyImageView(device.device(), view, nullptr);
                    }
                }
                swapChainImageViews.clear();
            }
        }

        void destroyDepthImageviews() {
            if (!depthImageViews.empty()) {
                for (auto view  : depthImageViews) {
                    if (view  != VK_NULL_HANDLE) {
                        vkDestroyImageView(device.device(), view, nullptr);
                    }
                }
                depthImageViews.clear();
            }
        }

        void destroySwapChainImages() {
            if (!swapChainImages.empty()) {
                for (auto image  : swapChainImages) {
                    if (image  != VK_NULL_HANDLE) {
                        vkDestroyImage(device.device(), image, nullptr);
                    }
                }
                depthImageViews.clear();
            }
        }

        void destroyDepthImages() {
            if (!depthImages.empty()) {
                for (auto image  : depthImages) {
                    if (image  != VK_NULL_HANDLE) {
                        vkDestroyImage(device.device(), image, nullptr);
                    }
                }
                depthImages.clear();
            }
        }

        void destroyDepthMemorys() {
            if (!depthImageMemorys.empty()) {
                for (auto memory  : depthImageMemorys) {
                    if (memory != VK_NULL_HANDLE) {
                        vkFreeMemory(device.device(), memory, nullptr);
                    }
                }
                depthImageMemorys.clear();
            }
        }

		void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();

        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass = VK_NULL_HANDLE;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        Device& device;
        VkExtent2D windowExtent;

        VkSwapchainKHR _swapChain;
		Ref<SwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores; // par frame
        std::vector<VkSemaphore> renderFinishedSemaphores; // par image
        std::vector<VkFence> inFlightFences; // par frame
        std::vector<VkFence> imagesInFlight; // par image
	    std::vector<VkSemaphore> renderFinishedSemaphoresPerImage;

        size_t currentFrame = 0;
    };
}