#ifndef VECTRIXWORKSPACE_SHADERTYPE_H
#define VECTRIXWORKSPACE_SHADERTYPE_H

#include "shaderc/shaderc.h"

typedef enum {
    Vertex_Shader,
    Fragment_Shader
} ShaderType;

inline shaderc_shader_kind shaderTypeToShaderCKind(ShaderType t) {
    switch (t) {
        case Vertex_Shader:   return shaderc_vertex_shader;
        case Fragment_Shader: return shaderc_fragment_shader;
        default:
            VC_CORE_ERROR("Unknown shader type");
    }
}

inline const char* toString(ShaderType t) {
    switch (t) {
        case Vertex_Shader:   return "Vertex shader";
        case Fragment_Shader: return "Fragment shader";
        default:
            VC_CORE_ERROR("Unknown shader type");
    }
}

#endif //VECTRIXWORKSPACE_SHADERTYPE_H