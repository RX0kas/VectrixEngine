#ifndef VECTRIXWORKSPACE_IMAGEFORMAT_H
#define VECTRIXWORKSPACE_IMAGEFORMAT_H
#include <string>
#include "volk.h"

namespace Vectrix {
    enum VectrixImageFormat {
        UNDEFINED,
        BGRA_SRGB,
        R_UNORM,
        RGBA_UNORM,
        R_SRGB,
        D32_SFLOAT,
        D32_SFLOAT_S8_UINT,
        D24_UNORM_S8_UINT
    };

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
