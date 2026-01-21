#include "Shader.h"

#include "Vectrix/Renderer/Renderer.h"
#include "GraphicAPI/Vulkan/rendering/VulkanShader.h"

namespace Vectrix {
    Shader* Shader::create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,const ShaderUniformLayout& layout,const BufferLayout& buffer_layout)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
            case RendererAPI::API::Vulkan:  return new VulkanShader(name, vertexPath, fragmentPath,layout,buffer_layout);
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
    }
}
