#ifndef VECTRIXWORKSPACE_VULKANSHADER_H
#define VECTRIXWORKSPACE_VULKANSHADER_H

#include "Vectrix/Renderer/Shaders/Shader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Pipeline.h"
#include "Vectrix/Renderer/Shaders/ShaderUniformLayout.h"

namespace Vectrix {
    class VulkanShader final : public Shader {
    public:
        VulkanShader(std::string  name, const std::string& vertexPath, const std::string& fragmentPath,const ShaderUniformLayout& layout, BufferLayout buffer_layout);
        ~VulkanShader() override;
        void bind() const override;
        void unbind() const override;
        void setUniformBool(const std::string& name,bool value) const override;
        void setUniform1i(const std::string &name, int value) const override;
        void setUniform1u(const std::string &name, unsigned int value) const override;
        void setUniform1f(const std::string &name, float value) const override;
        void setUniform2f(const std::string &name, glm::vec2 value) const override;
        void setUniform3f(const std::string &name, glm::vec3 value) const override;
        void setUniform4f(const std::string &name, glm::vec4 value) const override;
        void setUniformMat4f(const std::string &name, glm::mat4 value) const override;
        void sentCameraUniform(const PerspectiveCamera &camera) const override;
        void setModelMatrix(const glm::mat4& model) const override;
    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass, const std::string& vertexSrc, const std::string& fragmentSrc, BufferLayout layout);
    private:
        Device& m_device;
        VulkanRenderer& m_renderer;
        Ref<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout{};
        Own<SSBO> m_ssbo;
        Own<ShaderUniformLayout> m_layout;

        bool m_enable = true;
        friend class VulkanContext;
        friend class ShaderManager;
        friend class VulkanRenderer;
        // Info
        const std::string m_name;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANSHADER_H