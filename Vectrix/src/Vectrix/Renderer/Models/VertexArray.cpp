#include "VertexArray.h"

#include "../Renderer.h"
#include "GraphicAPI/Vulkan/rendering/VulkanVertexArray.h"

namespace Vectrix {
    VertexArray* VertexArray::create() {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
            case RendererAPI::API::Vulkan:  return new VulkanVertexArray();
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
    }
} // Vectrix