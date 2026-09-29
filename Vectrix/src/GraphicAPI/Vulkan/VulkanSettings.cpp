#include "vcpch.h"
#include "VulkanSettings.h"

#include <algorithm>
#include <cctype>

#include "Vectrix/Core/Log.h"
#include "Vectrix/Settings/SettingsManager.h"

namespace Vectrix {
    namespace {
        std::string toLower(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return s;
        }

        VkPresentModeKHR parsePresentMode(const std::string& raw, VkPresentModeKHR fallback) {
            const std::string s = toLower(raw);
            if (s == "fifo")        return VK_PRESENT_MODE_FIFO_KHR;
            if (s == "mailbox")     return VK_PRESENT_MODE_MAILBOX_KHR;
            if (s == "immediate")   return VK_PRESENT_MODE_IMMEDIATE_KHR;
            if (s == "fiforelaxed") return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
            VC_CORE_WARN("Unknown vulkan.swapchain.presentMode '{}', keeping default", raw);
            return fallback;
        }

        VkFilter parseFilter(const std::string& raw, VkFilter fallback) {
            const std::string s = toLower(raw);
            if (s == "linear")  return VK_FILTER_LINEAR;
            if (s == "nearest") return VK_FILTER_NEAREST;
            VC_CORE_WARN("Unknown vulkan.textures.filter '{}', keeping default", raw);
            return fallback;
        }

        VkSamplerAddressMode parseWrap(const std::string& raw, VkSamplerAddressMode fallback) {
            const std::string s = toLower(raw);
            if (s == "repeat")         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            if (s == "mirroredrepeat") return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            if (s == "clampedge")      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            if (s == "clampborder")    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            VC_CORE_WARN("Unknown vulkan.textures.wrapMode '{}', keeping default", raw);
            return fallback;
        }

        VkDebugUtilsMessageSeverityFlagsEXT parseSeverity(const std::string& raw,
                                                          VkDebugUtilsMessageSeverityFlagsEXT fallback) {
            constexpr auto e = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            constexpr auto w = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            constexpr auto i = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
            constexpr auto v = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

            const std::string s = toLower(raw);
            if (s == "error")   return e;
            if (s == "warning") return w | e;
            if (s == "info")    return i | w | e;
            if (s == "verbose") return v | i | w | e;
            VC_CORE_WARN("Unknown vulkan.device.validationLayerSeverity '{}', keeping default", raw);
            return fallback;
        }
    }

    const char* presentModeToString(VkPresentModeKHR mode) {
        switch (mode) {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:    return "immediate";
            case VK_PRESENT_MODE_MAILBOX_KHR:      return "mailbox";
            case VK_PRESENT_MODE_FIFO_KHR:         return "fifo";
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return "fifoRelaxed";
            default:                               return "unknown";
        }
    }

    VulkanSettings VulkanSettings::load() {
        VulkanSettings s;

        const JsonObject& root = SettingsManager::getSettings();
        const auto engineIt = root.find("engine");
        if (engineIt == root.end())
            return s;

        // const operator[] returns a null sentinel for any missing link, so this whole
        // chain is safe even when the settings file only defines part of the tree.
        const JsonValue& vk = engineIt->second["graphicAPI"]["vulkan"];

        const JsonValue& dp = vk["startingDescriptorPoolConfig"];
        s.descriptorPool.uboCount     = dp["uboCount"].getAs<uint32_t>().value_or(s.descriptorPool.uboCount);
        s.descriptorPool.ssboCount    = dp["ssboCount"].getAs<uint32_t>().value_or(s.descriptorPool.ssboCount);
        s.descriptorPool.samplerCount = dp["samplerCount"].getAs<uint32_t>().value_or(s.descriptorPool.samplerCount);
        s.descriptorPool.maxSets      = dp["maxSets"].getAs<uint32_t>().value_or(s.descriptorPool.maxSets);

        const JsonValue& sc = vk["swapchain"];
        if (const auto pm = sc["presentMode"].getAs<std::string>())
            s.swapchain.presentMode = parsePresentMode(*pm, s.swapchain.presentMode);
        s.swapchain.imageCount = sc["imageCount"].getAs<uint32_t>().value_or(s.swapchain.imageCount);

        const JsonValue& dev = vk["device"];
        s.device.preferDiscreteGpu = dev["preferDiscreteGpu"].getAs<bool>().value_or(s.device.preferDiscreteGpu);
        s.device.preferredGpuName  = dev["preferredGpuName"].getAs<std::string>().value_or(s.device.preferredGpuName);
        s.device.validationLayers  = dev["validationLayers"].getAs<bool>().value_or(s.device.validationLayers);
        if (const auto sev = dev["validationLayerSeverity"].getAs<std::string>())
            s.device.validationSeverity = parseSeverity(*sev, s.device.validationSeverity);

        const JsonValue& tex = vk["textures"];
        if (const auto f = tex["filter"].getAs<std::string>())
            s.textures.filter = parseFilter(*f, s.textures.filter);
        if (const auto w = tex["wrapMode"].getAs<std::string>())
            s.textures.wrapMode = parseWrap(*w, s.textures.wrapMode);
        s.textures.anisotropy = static_cast<float>(tex["anisotropy"].getAs<double>().value_or(s.textures.anisotropy));

        s.shaders.optimize = vk["shaders"]["optimize"].getAs<bool>().value_or(s.shaders.optimize);

        const JsonValue& rnd = vk["rendering"];
        s.rendering.backfaceCulling = rnd["backfaceCulling"].getAs<bool>().value_or(s.rendering.backfaceCulling);
        s.rendering.wireframe       = rnd["wireframe"].getAs<bool>().value_or(s.rendering.wireframe);

        return s;
    }
}
