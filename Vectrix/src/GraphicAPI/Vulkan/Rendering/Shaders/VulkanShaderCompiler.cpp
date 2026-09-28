#include "VulkanShaderCompiler.h"

#include <fstream>

#include "shaderc/shaderc.hpp"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
    VulkanShaderCompiler::VulkanShaderCompiler() {
        VC_PROFILER_FUNCTION();
        // Options
        m_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        m_options.SetSourceLanguage(shaderc_source_language_glsl);
        m_options.SetTargetSpirv(shaderc_spirv_version_1_6);
        m_options.SetOptimizationLevel(shaderc_optimization_level_zero);
        m_options.SetInvertY(false);
    }

    std::vector<uint32_t> VulkanShaderCompiler::compile_file(const char *src_name, ShaderType type,const char *src,bool optimize) {
        VC_PROFILER_FUNCTION();
        VC_CORE_INFO("Compiling a {} called {}, with{} optimization",toString(type),src_name,optimize ? "" : "out");

        shaderc_shader_kind kind = shaderTypeToShaderCKind(type);

        if (optimize) m_options.SetOptimizationLevel(shaderc_optimization_level_performance);
        else m_options.SetOptimizationLevel(shaderc_optimization_level_zero);

        // Errors are returned, not aborted on: a shader with a typo is user content (an asset dropped in the
        // editor, a project file) and must not take the whole application down
        const shaderc::PreprocessedSourceCompilationResult preprocessed = m_compiler.PreprocessGlsl(src, kind, src_name, m_options);
        if (preprocessed.GetCompilationStatus() != shaderc_compilation_status_success) {
            VC_CORE_ERROR_NO_EXIT("Preprocessing error in {}: {}", src_name, preprocessed.GetErrorMessage());
            return {};
        }

        const shaderc::SpvCompilationResult module = m_compiler.CompileGlslToSpv(std::string(preprocessed.cbegin(), preprocessed.cend()), kind, src_name, m_options);
        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            VC_CORE_ERROR_NO_EXIT("Compilation error in {}: {}", src_name, module.GetErrorMessage());
            return {};
        }

        return {module.cbegin(), module.cend()};
    }
} // Vectrix