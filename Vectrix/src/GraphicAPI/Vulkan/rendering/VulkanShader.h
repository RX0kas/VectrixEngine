#ifndef VECTRIXWORKSPACE_VULKANSHADER_H
#define VECTRIXWORKSPACE_VULKANSHADER_H

#include "Vectrix/Renderer/Shaders/ShaderManager.h"
#include "Vectrix/Renderer/Shaders/Shader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Pipeline.h"
#include "Vectrix/Renderer/Shaders/ShaderUniformLayout.h"

namespace Vectrix {
    struct TestSSBO {
        float time;
        float _pad[3]; // alignement 16
    };

    static_assert(sizeof(TestSSBO)%16==0);

    class Shader;

    class VulkanShader final : public Shader {
    public:
        VulkanShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,ShaderUniformLayout layout, BufferLayout buffer_layout);
        ~VulkanShader() override;
        void bind() const override;
        void unbind() const override;
        void setUniform1i(const std::string &name, int value) const override;
        void setUniform1f(const std::string &name, float value) const override;
        void setUniform2f(const std::string &name, glm::vec2 value) const override;
        void setUniform3f(const std::string &name, glm::vec3 value) const override;
        void setUniform4f(const std::string &name, glm::vec4 value) const override;

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass, const std::string& vertexSrc, const std::string& fragmentSrc, BufferLayout layout);
    private:
        Device& m_device;
        VulkanRenderer& m_renderer;
        ShaderUniformLayout m_layout;
        Ref<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout{};
        Scope<SSBO> m_ssbo;

        bool m_enable = true;
        friend class VulkanContext;
        friend class ShaderManager;
        friend class VulkanRenderer;
        // Info
        const std::string m_name;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANSHADER_H