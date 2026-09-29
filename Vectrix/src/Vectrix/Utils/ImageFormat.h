#ifndef VECTRIXWORKSPACE_IMAGEFORMAT_H
#define VECTRIXWORKSPACE_IMAGEFORMAT_H
#include <string>
#include "volk.h"

/**
 * @file ImageFormat.h
 * @brief The image formats the engine exposes, and their translation to Vulkan
 * @ingroup utils
 */

namespace Vectrix {
    /**
     * @brief How the pixels of an image are laid out
     *
     * This is the engine side of the format, so the rest of the code does not have to
     * name a Vulkan value. Use toVulkanFormat and toVectrixFormat to cross the boundary.
     * @see Framebuffer
     * @see Texture
     * @ingroup utils
     */
    enum VectrixImageFormat {
        UNDEFINED,          ///< No format, or one the engine does not expose
        BGRA_SRGB,          ///< Four 8 bit channels in blue green red alpha order, sRGB encoded
        R_UNORM,            ///< One 8 bit channel, normalised to the 0 to 1 range
        RGBA_UNORM,         ///< Four 8 bit channels, normalised to the 0 to 1 range
        R_SRGB,             ///< One 8 bit channel, sRGB encoded
        D32_SFLOAT,         ///< A 32 bit float depth buffer
        D32_SFLOAT_S8_UINT, ///< A 32 bit float depth buffer with an 8 bit stencil
        D24_UNORM_S8_UINT   ///< A 24 bit normalised depth buffer with an 8 bit stencil
    };

    /**
     * @brief Turn a format into its name, for logging
     * @param f The format to describe
     * @return The name of the value, or a fallback when it is not a known one
     * @ingroup utils
     */
    inline std::string toString(VectrixImageFormat f) {
        switch (f) {
            case UNDEFINED: return "UNDEFINED";
            case BGRA_SRGB: return "BGRA_SRGB";
            case R_UNORM: return "R_UNORM";
            case RGBA_UNORM: return "RGBA_UNORM";
            case R_SRGB: return "R_SRGB";
            case D32_SFLOAT: return "D32_SFLOAT";
            case D32_SFLOAT_S8_UINT: return "D32_SFLOAT_S8_UINT";
            case D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
        }
        return "UNKNOWN_IMAGE_FORMAT";
    }

    /**
     * @brief Translate an engine format into the Vulkan one
     * @param f The engine format
     * @return The matching Vulkan format, `VK_FORMAT_UNDEFINED` when there is none
     * @see toVectrixFormat
     * @ingroup utils
     */
    inline VkFormat toVulkanFormat(VectrixImageFormat f) {
        switch (f) {
            case UNDEFINED: return VK_FORMAT_UNDEFINED;
            case RGBA_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
            case BGRA_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
            case R_UNORM: return VK_FORMAT_R8_UNORM;
            case R_SRGB: return VK_FORMAT_R8_SRGB;
            case D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
            case D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
            default: return VK_FORMAT_UNDEFINED;
        }
    }

    /**
     * @brief Translate a Vulkan format into the engine one
     * @param f The Vulkan format
     * @return The matching engine format, #UNDEFINED when the engine does not expose it
     * @see toVulkanFormat
     * @ingroup utils
     */
    inline VectrixImageFormat toVectrixFormat(VkFormat f) {
        switch (f) {
            case VK_FORMAT_UNDEFINED: return UNDEFINED;
            case VK_FORMAT_R8G8B8A8_UNORM: return RGBA_UNORM;
            case VK_FORMAT_B8G8R8A8_SRGB: return BGRA_SRGB;
            case VK_FORMAT_R8_UNORM: return R_UNORM;
            case VK_FORMAT_R8_SRGB: return R_SRGB;
            case VK_FORMAT_D32_SFLOAT: return D32_SFLOAT;
            case VK_FORMAT_D32_SFLOAT_S8_UINT: return D32_SFLOAT_S8_UINT;
            case VK_FORMAT_D24_UNORM_S8_UINT: return D24_UNORM_S8_UINT;
            default: return UNDEFINED;
        }
    }
}

#endif //VECTRIXWORKSPACE_IMAGEFORMAT_H
