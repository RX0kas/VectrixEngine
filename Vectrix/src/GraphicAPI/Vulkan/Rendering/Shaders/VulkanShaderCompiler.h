#ifndef VECTRIXWORKSPACE_SHADERCOMPILER_H
#define VECTRIXWORKSPACE_SHADERCOMPILER_H
#include <string>
#include <unordered_map>
#include <vector>

#include "shaderc/shaderc.h"
#include "shaderc/shaderc.hpp"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Renderer/Shaders/ShaderType.h"
#include "Vectrix/Utils/Hashing.h"

namespace Vectrix {
    class VulkanShaderCompiler {
    public:
        VulkanShaderCompiler();

        void addMacro(const std::string& name,const std::string& value) {
            p_macros.emplace(name,value);
        }
        std::string getMacro(std::string name) {
            auto it = p_macros.find(name);
            if (it == p_macros.end()) {
                VC_CORE_ERROR("Shader with the name \"{}\" doesn't exist", name);
            }
            return it->second;
        }
        std::vector<uint32_t> compile_file(const char *src_name,ShaderType type,const char *src,bool optimize = false);
    private:
        std::string preprocessing(const char *source_name, const shaderc_shader_kind& kind, const char *src);
    private:
        std::unordered_map<std::string,std::string,XXH3> p_macros;
        shaderc::Compiler m_compiler;
        shaderc::CompileOptions m_options;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SHADERCOMPILER_H