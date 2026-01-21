#ifndef VECTRIXWORKSPACE_SHADERUNIFORM_H
#define VECTRIXWORKSPACE_SHADERUNIFORM_H

#include <cstdint>

namespace Vectrix {
    enum class ShaderUniformType {
        Int,
        Float,
        Vec2,
        Vec3,
        Vec4, // TODO: implement all of the other
        Mat4,
        Uint,
        Bool
    };

    // renvoie l'alignement en bytes (std430 rules simplified)
    inline uint32_t UniformStd430Alignment(ShaderUniformType t) {
        switch (t) {
            case ShaderUniformType::Float: return 4;
            case ShaderUniformType::Int:   return 4;
            case ShaderUniformType::Uint:  return 4;
            case ShaderUniformType::Bool:  return 4;
            case ShaderUniformType::Vec2:  return 8;
            case ShaderUniformType::Vec3:  // vec3 occupies vec4 slot
            case ShaderUniformType::Vec4:  return 16;
            case ShaderUniformType::Mat4:  return 16;
            default: return 4;
        }
    }

    // renvoie la "taille utile" (nombre d'octets réellement copiés)
    // vec3 sera 12 bytes de données mais occupe 16 bytes d'espace
    inline uint32_t UniformSizeInBytes(ShaderUniformType t) {
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