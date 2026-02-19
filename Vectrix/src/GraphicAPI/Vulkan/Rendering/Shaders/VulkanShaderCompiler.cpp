#include "VulkanShaderCompiler.h"

#include <fstream>

#include "shaderc/shaderc.hpp"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    std::string VulkanShaderCompiler::preprocessing(const char *source_name, const shaderc_shader_kind& kind, const char *src) {
        shaderc::PreprocessedSourceCompilationResult result = m_compiler.PreprocessGlsl(src, kind, source_name, m_options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            VC_CORE_CRITICAL("Preprocessing error: {}",result.GetErrorMessage());
        }

        return {result.cbegin(), result.cend()};
    }

    VulkanShaderCompiler::VulkanShaderCompiler() {
        p_macros.reserve(256);
        p_macros.max_load_factor(0.7f);
        // Options
        m_options.SetTargetEnvironment(shaderc_target_env_vulkan,shaderc_env_version_vulkan_1_2);
        m_options.SetSourceLanguage(shaderc_source_language_glsl);
        m_options.SetOptimizationLevel(shaderc_optimization_level_zero);
        m_options.SetTargetSpirv(shaderc_spirv_version_1_2);
        m_options.SetForcedVersionProfile(450, shaderc_profile_none);
        m_options.SetInvertY(false);
    }

    std::vector<uint32_t> VulkanShaderCompiler::compile_file(const char *src_name,ShaderType type,const char *src,bool optimize) {
        VC_CORE_INFO("Compiling a {} called {}, with{} optimization",toString(type),src_name,optimize ? "" : "out");

        shaderc_shader_kind kind = shaderTypeToShaderCKind(type);

        for (const auto& macro : p_macros) {
            m_options.AddMacroDefinition(macro.first,macro.second);
        }

        if (optimize) m_options.SetOptimizationLevel(shaderc_optimization_level_performance);
        else m_options.SetOptimizationLevel(shaderc_optimization_level_zero);

        std::string preprocessed = preprocessing(src_name,kind,src);
        shaderc::SpvCompilationResult module = m_compiler.CompileGlslToSpv(preprocessed, kind, src_name, m_options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            VC_CORE_CRITICAL("Compilation error: {}", module.GetErrorMessage());
        }

        return {module.cbegin(), module.cend()};
    }
} // Vectrix