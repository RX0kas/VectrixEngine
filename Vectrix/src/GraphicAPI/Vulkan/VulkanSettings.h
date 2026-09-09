#pragma once
#include <cstdint>
#include <string>

#include "volk.h"

/**
 * @file VulkanSettings.h
 * @brief Typed view of the Vulkan backend options read from the project settings
 * @ingroup settings
 *
 * Populated once from @c engine.graphicAPI.vulkan.* at the start of
 * VulkanContext::init(), then read by the backend instead of hardcoded values. Every
 * field defaults to the value the engine used before settings existed, so an absent or
 * empty settings file changes nothing.
 */

namespace Vectrix {
    struct VulkanSettings {
        struct DescriptorPool {
            uint32_t uboCount = 256;
            uint32_t ssboCount = 256;
            uint32_t samplerCount = 4096;
            uint32_t maxSets = 512;
        } descriptorPool;

        struct Swapchain {
            VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            uint32_t imageCount = 0; ///< 0 = automatic (minImageCount + 1)
        } swapchain;

        struct DeviceCfg {
            bool preferDiscreteGpu = true;
            std::string preferredGpuName; ///< substring match on VkPhysicalDeviceProperties::deviceName, empty = ignore
            bool validationLayers = true;
            VkDebugUtilsMessageSeverityFlagsEXT validationSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        } device;

        struct Textures {
            VkFilter filter = VK_FILTER_LINEAR;
            VkSamplerAddressMode wrapMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            float anisotropy = 16.0f; ///< 0 = disabled; clamped to the device limit when used
        } textures;

        struct Shaders {
            bool optimize = true;
        } shaders;

        struct Rendering {
            bool backfaceCulling = true;
            bool wireframe = false;
        } rendering;

        /**
         * @brief Build the settings from the currently loaded SettingsManager tree.
         * @return A fully populated struct; missing keys keep their default.
         */
        static VulkanSettings load();
    };

    /**
     * @brief Human-readable name for a present mode, for logging.
     */
    const char* presentModeToString(VkPresentModeKHR mode);
}
