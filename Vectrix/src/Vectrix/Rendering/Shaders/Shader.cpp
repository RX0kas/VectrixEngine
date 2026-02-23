#include "Shader.h"

#include "Vectrix/Rendering/Renderer.h"
#include "../../../GraphicAPI/Vulkan/Rendering/Shaders/VulkanShader.h"

namespace Vectrix {
    Shader* Shader::create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,ShaderUniformLayout layout,const BufferLayout& buffer_layout,bool affectedByCamera) {
        if (affectedByCamera) {
            layout.add("cameraTransform",ShaderUniformType::Mat4);
            layout.add("modelMat",ShaderUniformType::Mat4);
        }

        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
            case RendererAPI::API::Vulkan:  return new VulkanShader(name, vertexPath, fragmentPath,layout,buffer_layout,affectedByCamera);
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
    }
}
