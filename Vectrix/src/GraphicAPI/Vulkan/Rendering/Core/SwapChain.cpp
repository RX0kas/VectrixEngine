#include "vcpch.h"
#include "SwapChain.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"


namespace Vectrix {
    SwapChain::SwapChain(Device& deviceRef, VkExtent2D extent)
        : m_device{ deviceRef }, m_windowExtent{ extent } {
        init();
    }

    SwapChain::SwapChain(Device& deviceRef, VkExtent2D extent, std::shared_ptr<SwapChain> previous)
        : m_device{ deviceRef }, m_windowExtent{ extent }, m_oldSwapChain{ std::move(previous) } {
        init();

        // clean up old swap chain because we no longer need it
        m_oldSwapChain = nullptr;
    }

    SwapChain::~SwapChain() {
        cleanup();
    }


    void SwapChain::init() {
        // No render pass/framebuffers: the renderer and ImGui draw with dynamic rendering straight into the image views
        createSwapChain();
        createImageViews();
        createDepthResources();
        createSyncObjects();
    }

    void SwapChain::cleanup() {
        vkDeviceWaitIdle(m_device.device());

        for (const auto imageView : m_swapChainImageViews) {
            vkDestroyImageView(m_device.device(), imageView, nullptr);
        }
        m_swapChainImageViews.clear();

        if (m_swapChain != nullptr) {
            vkDestroySwapchainKHR(m_device.device(), m_swapChain, nullptr);
            m_swapChain = nullptr;
        }

        for (size_t i = 0; i < m_depthImages.size(); i++) {
            vkDestroyImageView(m_device.device(), m_depthImageViews[i], nullptr);
            m_device.destroyImage(m_depthImages[i], m_depthImageAllocations[i]);   // ← VMA
        }
        m_depthImages.clear();
        m_depthImageAllocations.clear();
        m_depthImageViews.clear();

        for (auto& sem : m_renderFinishedSemaphores) {
            if (sem != VK_NULL_HANDLE) {
                vkDestroySemaphore(m_device.device(), sem, nullptr);
                sem = VK_NULL_HANDLE;
            }
        }
        m_renderFinishedSemaphores.clear();

        for (const auto semaphore : m_imageAvailableSemaphores) {
            vkDestroySemaphore(m_device.device(), semaphore, nullptr);
        }
        m_imageAvailableSemaphores.clear();

        for (const auto fence : m_inFlightFences) {
            vkDestroyFence(m_device.device(), fence, nullptr);
        }
        m_inFlightFences.clear();
    }

    VkResult SwapChain::acquireNextImage(uint32_t* imageIndex) const {
        // The GPU must be done with this frame slot before the CPU touches it again: its command buffer, its image
        // available semaphore, and the per-frame object SSBO/indirect buffers the renderer is about to overwrite.
        // The image itself needs no wait here: the GPU work writing it waits on the acquire semaphore
        if (vkWaitForFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, VC_TIMEOUT_SYNC) == VK_TIMEOUT) {
            VC_CORE_CRITICAL("GPU HANG DETECTED - frame {} fence never signaled", m_currentFrame);
        }

        return vkAcquireNextImageKHR(m_device.device(), m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, imageIndex);
    }

    VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex) {
        VC_CORE_ASSERT(buffers != nullptr, "buffers is null");
        VC_CORE_ASSERT(imageIndex != nullptr, "imageIndex is null");
        VC_CORE_ASSERT(*imageIndex < m_swapChainImages.size(), "imageIndex {} out of bounds (max {})", *imageIndex, m_swapChainImages.size());
        VC_CORE_ASSERT(m_currentFrame < MAX_FRAMES_IN_FLIGHT, "currentFrame {} out of bounds", m_currentFrame);
        VC_CORE_ASSERT(m_imageAvailableSemaphores[m_currentFrame] != VK_NULL_HANDLE, "imageAvailableSemaphore is null");
        VC_CORE_ASSERT(m_renderFinishedSemaphores[*imageIndex] != VK_NULL_HANDLE, "renderFinishedSemaphore is null for imageIndex {}", *imageIndex);
        VC_CORE_ASSERT(m_inFlightFences[m_currentFrame] != VK_NULL_HANDLE, "inFlightFence is null");
        VC_CORE_ASSERT(m_swapChain != VK_NULL_HANDLE, "swapchain handle is null");

        // acquireNextImage already waited on this frame's fence
        vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);

        const VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        const VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[*imageIndex] };
        constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
            VC_CORE_CRITICAL("vkQueueSubmit failed");
        }

        // No wait on the fence here: with FIFO the GPU can't write the image before the display releases it
        // (the acquire semaphore), so blocking until the work finishes lined the CPU up with the vblank and cost
        // a whole refresh every other frame (72/144 fps at 144 Hz). A hang is still caught one frame later,
        // by acquireNextImage waiting on this same fence with VC_TIMEOUT_SYNC

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain;
        presentInfo.pImageIndices = imageIndex;

        return vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);
    }


    void SwapChain::createSwapChain() {
        auto [capabilities, formats, presentModes] = m_device.getSwapChainSupport();

        const VulkanSettings::Swapchain& scSettings = VulkanContext::instance().settings().swapchain;

        const auto [format, colorSpace] = chooseSwapSurfaceFormat(formats);
        const VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes, scSettings.presentMode);
        const VkExtent2D extent = chooseSwapExtent(capabilities);

        uint32_t imageCount = scSettings.imageCount != 0 ? scSettings.imageCount : capabilities.minImageCount + 1;

        imageCount = std::max(imageCount, capabilities.minImageCount);
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }



        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_device.surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format;
        createInfo.imageColorSpace = colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const QueueFamilyIndices indices = m_device.findPhysicalQueueFamilies();
        const uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = m_oldSwapChain == nullptr ? VK_NULL_HANDLE : m_oldSwapChain->m_swapChain;


        VkResult result = vkCreateSwapchainKHR(m_device.device(), &createInfo, nullptr, &m_swapChain);
        if (result != VK_SUCCESS) {
            VC_CORE_CRITICAL("vkCreateSwapchainKHR failed with code: {0}", string_VkResult(result));
        }

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = format;
        m_swapChainExtent = extent;
        VC_CORE_INFO("Swap chain: {} images of {}x{} (surface min {} / max {})", imageCount, extent.width, extent.height, capabilities.minImageCount, capabilities.maxImageCount);
    }

    void SwapChain::createImageViews() {
        m_swapChainImageViews.resize(m_swapChainImages.size());
        for (size_t i = 0; i < m_swapChainImages.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_swapChainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
                VC_CORE_CRITICAL("Failed to create texture image view");
            }
        }
    }

    void SwapChain::createDepthResources() {
        const VkFormat depthFormat = findDepthFormat();
        m_swapChainDepthFormat = depthFormat;
        const auto [width, height] = getSwapChainExtent();

        m_depthImages.resize(imageCount());
        m_depthImageAllocations.resize(imageCount());
        m_depthImageViews.resize(imageCount());

        for (int i = 0; i < m_depthImages.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImages[i], m_depthImageAllocations[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS) {
                VC_CORE_CRITICAL("failed to create texture image view!");
            }
        }
    }

    void SwapChain::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(m_swapChainImages.size());

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled so first vkWaitForFences returns immediately

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VC_VK_CHECK(vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]),"Failed to create imageAvailableSemaphores[{}]",i);
            VC_VK_CHECK(vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_inFlightFences[i]),"Failed to create inFlightFences[{}]",i);
        }

        for (uint32_t i = 0; i < imageCount(); i++) {
            VC_VK_CHECK(vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]),"Failed to create renderFinishedSemaphores[{}]",i);
        }
    }


    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR preferred) const {
        const auto isAvailable = [&](const VkPresentModeKHR mode) {
            return std::ranges::find(availablePresentModes, mode) != availablePresentModes.end();
        };
        // The setting and the surface don't change at runtime: report the choice once, not on every resize
        const bool report = m_oldSwapChain == nullptr;

        if (isAvailable(preferred)) {
            if (report) VC_CORE_INFO("Present mode: {}", presentModeToString(preferred));
            return preferred;
        }

        // Mailbox is asked for to render uncapped without tearing. XWayland (an X11 window in a Wayland session,
        // where NVIDIA offers no mailbox) hands finished frames to the compositor, so immediate doesn't tear there
        // either, while fifo would tie the frame rate to the compositor's pacing (72/144 fps swings at 144 Hz)
        const bool xWayland = glfwGetPlatform() == GLFW_PLATFORM_X11 && Application::instance().window().getDisplayServer() == WAYLAND;
        if (preferred == VK_PRESENT_MODE_MAILBOX_KHR && xWayland && isAvailable(VK_PRESENT_MODE_IMMEDIATE_KHR)) {
            if (report) VC_CORE_INFO("Present mode: immediate (mailbox isn't supported under XWayland, whose compositing keeps it from tearing)");
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        // FIFO is the only mode the spec guarantees is always available.
        if (report) VC_CORE_WARN("Present mode '{}' not supported by the surface, falling back to fifo (V-Sync)", presentModeToString(preferred));
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = m_windowExtent;
        actualExtent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;

    }

    VkFormat SwapChain::findDepthFormat() const {
        return m_device.findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}
