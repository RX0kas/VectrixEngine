#ifndef VECTRIXWORKSPACE_SHADERUNIFORM_H
#define VECTRIXWORKSPACE_SHADERUNIFORM_H

#include <cstdint>

namespace Vectrix {
    enum class ShaderUniformType {
        Int,
        Float,
        Vec2,
        Vec3,
        Vec4,
        Mat4,
        Uint,
        Bool
    };

    // renvoie l'alignement en bytes (std430 rules simplified)
    inline uint32_t uniformAlignment(ShaderUniformType t) {
        switch (t) {
            case ShaderUniformType::Float:
            case ShaderUniformType::Int:
            case ShaderUniformType::Uint:
            case ShaderUniformType::Bool:  return 4;
            case ShaderUniformType::Vec2:  return 8;
            case ShaderUniformType::Vec3:  // vec3 occupies vec4 slot
            case ShaderUniformType::Vec4:  return 16;
            case ShaderUniformType::Mat4:  return 16;
            default: return 4;
        }
    }

    inline uint32_t uniformSizeInBytes(ShaderUniformType t) {
        switch (t) {
            case ShaderUniformType::Float: return 4;
            case ShaderUniformType::Int:   return 4;
            case ShaderUniformType::Uint:  return 4;
            case ShaderUniformType::Bool:  return 4;
            case ShaderUniformType::Vec2:  return 8;
            case ShaderUniformType::Vec3:  return 12;
            case ShaderUniformType::Vec4:  return 16;
            case ShaderUniformType::Mat4:  return 16 * 4;
            default: return 4;
        }
    }
}

#endif //VECTRIXWORKSPACE_SHADERUNIFORM_H