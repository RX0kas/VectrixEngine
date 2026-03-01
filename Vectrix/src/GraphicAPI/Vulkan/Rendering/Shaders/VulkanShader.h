#ifndef VECTRIXWORKSPACE_VULKANSHADER_H
#define VECTRIXWORKSPACE_VULKANSHADER_H

#include "Vectrix/Rendering/Shaders/Shader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "GraphicAPI/Vulkan/Rendering/Pipeline.h"
#include "Vectrix/Rendering/Shaders/ShaderUniformLayout.h"
#include "Vectrix/Rendering/Shaders/UniformTrait.h"

namespace Vectrix {
    class VulkanShader final : public Shader {
    public:
        VulkanShader(std::string  name, const std::string& vertexPath, const std::string& fragmentPath,const ShaderUniformLayout& layout, BufferLayout buffer_layout,bool affectedByCamera);
        ~VulkanShader() override;
        void bind() const override;
        void setUniformBool(const std::string& name,bool value) const override;
        void setUniform1i(const std::string &name, int value) const override;
        void setUniform1u(const std::string &name, unsigned int value) const override;
        void setUniform1f(const std::string &name, float value) const override;
        void setUniform2f(const std::string &name, glm::vec2 value) const override;
        void setUniform3f(const std::string &name, glm::vec3 value) const override;
        void setUniform4f(const std::string &name, glm::vec4 value) const override;
        void setUniformMat4f(const std::string &name, glm::mat4 value) const override;
        void sendCameraUniform(const glm::mat4& camera) const override;
        void setModelMatrix(const glm::mat4& model) const override;
        void setTexture(uint32_t index, Ref<Texture> texture) override;

        void setUniformImplementation(const std::string& name,ShaderUniformType type,const void* data,size_t size) const override {
            VC_VERIFY_UNIFORM_NAME(name);

            const auto* e = m_layout->find(name);
            if (!e) {
                VC_CORE_ERROR("{} is not found in layout", name);
            }

            if (e->type != type) {
                VC_CORE_ERROR("Uniform '{}' type mismatch (expected {}, got {})",name,static_cast<int>(e->type),static_cast<int>(type));
            }

            m_ssbo->copyToFrame(m_renderer.getFrameIndex(),e->offset,data,size);
        }

        [[nodiscard]] bool isAffectedByCamera() const override {return m_affectedByCamera;}

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

        std::string m_fragSRC;
        std::string m_vertSRC;

        bool m_enable = true;
        bool m_affectedByCamera;
        friend class VulkanContext;
        friend class ShaderManager;
        friend class Shader;
        friend class VulkanRenderer;
        // Info
        const std::string m_name;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANSHADER_H