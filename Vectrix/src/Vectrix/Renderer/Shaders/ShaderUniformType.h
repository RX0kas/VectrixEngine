#ifndef VECTRIXWORKSPACE_SHADERUNIFORM_H
#define VECTRIXWORKSPACE_SHADERUNIFORM_H

/**
 * @file ShaderUniformType.h
 * @brief Definition of how to describe the different types in the GPU
 * @ingroup shaders
 */

namespace Vectrix {
    /**
     * @brief This is an Enum that stores every type of data that can be sent to the GPU
     */
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

    /**
     * @brief This function returns the bytes alignment for the ShaderUniformType given (std430 rules)
     * @param t The ShaderUniformType whose alignment we need to obtain
     * @return The bytes alignment
     */
    inline uint32_t uniformAlignment(ShaderUniformType t) {
        switch (t) {
            case ShaderUniformType::Float:
            case ShaderUniformType::Int:
            case ShaderUniformType::Uint:
            case ShaderUniformType::Bool:  return 4;
            case ShaderUniformType::Vec2:  return 8;
            case ShaderUniformType::Vec3:  // vec3 occupies vec4 slot
            case ShaderUniformType::Vec4:
            case ShaderUniformType::Mat4:  return 16;
            default: return 4;
        }
    }
    /**
    * @brief This function returns the bytes size for the ShaderUniformType given (std430 rules)
    * @param t The ShaderUniformType whose size we need to obtain
    * @return The bytes size
    */
    inline uint32_t uniformSizeInBytes(ShaderUniformType t) {
        switch (t) {
            case ShaderUniformType::Float:
            case ShaderUniformType::Int:
            case ShaderUniformType::Uint:
            case ShaderUniformType::Bool:  return 4;
            case ShaderUniformType::Vec2:  return 8;
            case ShaderUniformType::Vec3:  return 12;
            case ShaderUniformType::Vec4:  return 16;
            case ShaderUniformType::Mat4:  return 16 * 4;
            default: return 4;
        }
    }

    /**
     * @brief This function returns the name of the ShaderUniformType given so it can be logged or for other purpose
     * @param t The ShaderUniformType whose name we need to obtain
     * @return A string with the name of the ShaderUniformType
     */
    constexpr const char* toString(ShaderUniformType t) {
        switch (t) {
            case ShaderUniformType::Bool:  return "Bool";
            case ShaderUniformType::Float: return "Float";
            case ShaderUniformType::Int: return "Int";
            case ShaderUniformType::Uint: return "UInt";
            case ShaderUniformType::Vec2: return "Vec2";
            case ShaderUniformType::Vec3: return "Vec3";
            case ShaderUniformType::Vec4: return "Vec4";
            case ShaderUniformType::Mat4: return "Mat4";
            default: return "Unknown Type";
        }
    }
}

#endif //VECTRIXWORKSPACE_SHADERUNIFORM_H