#ifndef VECTRIXWORKSPACE_SHADERCOMPILER_H
#define VECTRIXWORKSPACE_SHADERCOMPILER_H
#include <string>
#include <unordered_map>
#include <vector>

#include "shaderc/shaderc.h"
#include "shaderc/shaderc.hpp"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Utils/Hashing.h"

namespace Vectrix {

    typedef enum {
        VertexShader,
        FragmentShader
    } ShaderType;

    inline shaderc_shader_kind shaderTypeToShaderCKind(ShaderType t) {
        switch (t) {
            case VertexShader:   return shaderc_vertex_shader;
            case FragmentShader: return shaderc_fragment_shader;
            default:
                VC_CORE_ERROR("Unknown shader type");
        }
        return shaderc_anyhit_shader;
    }

    inline const char* toString(ShaderType t) {
        switch (t) {
            case VertexShader:   return "Vertex shader";
            case FragmentShader: return "Fragment shader";
            default:
                VC_CORE_ERROR("Unknown shader type");
        }
        return "not_found";
    }

    class VulkanShaderCompiler {
    public:
        VulkanShaderCompiler();

        /// @return The SPIR-V code, or an empty vector when the source doesn't compile (the error is logged)
        std::vector<uint32_t> compile_file(const char *src_name,ShaderType type,const char *src,bool optimize = false);
    private:
        shaderc::Compiler m_compiler;
        shaderc::CompileOptions m_options;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SHADERCOMPILER_H