#ifndef VECTRIXWORKSPACE_VULKANSHADER_H
#define VECTRIXWORKSPACE_VULKANSHADER_H

#include "Vectrix/Rendering/Shaders/Shader.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "GraphicAPI/Vulkan/Rendering/Data/ShaderSSBO.h"

namespace Vectrix {
    class Pipeline;

    class VulkanShader final : public Shader {
    public:
        /// Names the embedded outline shaders are registered under. No asset can take them: a shader asset's id
        /// is its path, which ends in .vcshader
        static constexpr const char* k_MaskShaderName = "builtin:mask";
        static constexpr const char* k_OutlineShaderName = "builtin:outline";

        VulkanShader(std::string name, const std::string& source,const ShaderUniformLayout& layout, BufferLayout buffer_layout,bool affectedByCamera);
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
        uint32_t useTexture(std::shared_ptr<Texture> texture) override;
        uint32_t useFramebuffer(std::shared_ptr<Framebuffer> framebuffer) override;

        void setUniformImplementation(const std::string& name,ShaderUniformType type,const void* data,size_t size) const override {
            VC_VERIFY_UNIFORM_NAME(name);

            const auto* e = m_layout->find(name);
            // VC_CORE_ERROR is compiled out in release: return explicitly rather than dereferencing a null entry
            if (!e) {
                VC_CORE_ERROR("{} is not found in layout", name);
                return;
            }

            if (e->type != type) {
                VC_CORE_ERROR("Uniform '{}' type mismatch (expected {}, got {})",name,static_cast<int>(e->type),static_cast<int>(type));
                return;
            }

            m_ssbo->copyToFrame(m_renderer.getFrameIndex(),e->offset,data,size);
        }

        [[nodiscard]] bool isAffectedByCamera() const override {return m_affectedByCamera;}

        [[nodiscard]] std::string getID() const override { return m_name; }

    private:
        void setID(std::string id) override { m_name = std::move(id); }

        void createPipelineLayout();
        void createPipeline(BufferLayout layout);
        /// Binds an image to a slot of u_Textures, keeping the slot for as long as owner (what the image belongs to) exists
        uint32_t useImage(const std::string& key, const VkDescriptorImageInfo& imageInfo, std::weak_ptr<const void> owner);
        /// Frees the slots of the textures/framebuffers destroyed since they were bound
        void reclaimImageSlots();
    private:
        Device& m_device;
        VulkanRenderer& m_renderer;
        std::shared_ptr<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout{};
        std::unique_ptr<ShaderSSBO> m_ssbo;
        std::unique_ptr<ShaderUniformLayout> m_layout;

        std::string m_fragSRC;
        std::string m_vertSRC;

        bool m_enable = true;
        bool m_affectedByCamera;
        friend class VulkanContext;
        friend class ShaderManager;
        friend class Shader;
        friend class VulkanRenderer;

        /// A slot of u_Textures and what its image belongs to
        struct ImageSlot {
            uint32_t index;
            std::weak_ptr<const void> owner;
        };
        Cache<std::string, ImageSlot> m_imageSlots;
        std::vector<uint32_t> m_freeImageSlots; ///< Slots given back by reclaimImageSlots
        /// Next never used slot. Slot 0 keeps the not_found texture every slot starts with: what an image gets when
        /// they are all taken
        uint32_t m_nextImageSlot = 1;
        bool m_imageSlotsFullReported = false; ///< The overflow is logged once, not on every frame

        // Info
        std::string m_name;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_VULKANSHADER_H
