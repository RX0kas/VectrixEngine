#include "VulkanShaderCompiler.h"

#include <fstream>

#include "shaderc/shaderc.hpp"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Threading/JobSystem.h"

namespace Vectrix {
    namespace {
        void setCommonOptions(shaderc::CompileOptions& options, const shaderc_optimization_level level) {
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
            options.SetSourceLanguage(shaderc_source_language_glsl);
            options.SetTargetSpirv(shaderc_spirv_version_1_6);
            options.SetOptimizationLevel(level);
            options.SetInvertY(false);
        }
    }

    VulkanShaderCompiler::VulkanShaderCompiler() {
        VC_PROFILER_FUNCTION();
        // Two option sets set once, instead of changing the level on each call: compile_file runs on several threads
        setCommonOptions(m_options, shaderc_optimization_level_zero);
        setCommonOptions(m_optimizedOptions, shaderc_optimization_level_performance);
    }

    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> VulkanShaderCompiler::compileStages(const char* name, const char* vertexSource,
                                                                                         const char* fragmentSource, const bool optimize) const {
        VC_PROFILER_FUNCTION();
        std::vector<uint32_t> vertex;
        std::vector<uint32_t> fragment;
        auto compileFragment = [&] { fragment = compile_file(name, FragmentShader, fragmentSource, optimize); };
        if (JobSystem::exists()) {
            JobSystem& jobs = JobSystem::get();
            const JobHandle fragmentJob = jobs.submit(compileFragment);
            vertex = compile_file(name, VertexShader, vertexSource, optimize);
            jobs.wait(fragmentJob);
        } else {
            vertex = compile_file(name, VertexShader, vertexSource, optimize);
            compileFragment();
        }
        return {std::move(vertex), std::move(fragment)};
    }

    std::vector<uint32_t> VulkanShaderCompiler::compile_file(const char *src_name, ShaderType type,const char *src,bool optimize) const {
        VC_PROFILER_FUNCTION();
        VC_CORE_INFO("Compiling a {} called {}, with{} optimization",toString(type),src_name,optimize ? "" : "out");

        shaderc_shader_kind kind = shaderTypeToShaderCKind(type);
        const shaderc::CompileOptions& options = optimize ? m_optimizedOptions : m_options;

        // Errors are returned, not aborted on: a shader with a typo is user content (an asset dropped in the
        // editor, a project file) and must not take the whole application down
        const shaderc::PreprocessedSourceCompilationResult preprocessed = m_compiler.PreprocessGlsl(src, kind, src_name, options);
        if (preprocessed.GetCompilationStatus() != shaderc_compilation_status_success) {
            VC_CORE_ERROR_NO_EXIT("Preprocessing error in {}: {}", src_name, preprocessed.GetErrorMessage());
            return {};
        }

        const shaderc::SpvCompilationResult module = m_compiler.CompileGlslToSpv(std::string(preprocessed.cbegin(), preprocessed.cend()), kind, src_name, options);
        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            VC_CORE_ERROR_NO_EXIT("Compilation error in {}: {}", src_name, module.GetErrorMessage());
            return {};
        }

        return {module.cbegin(), module.cend()};
    }
} // Vectrix