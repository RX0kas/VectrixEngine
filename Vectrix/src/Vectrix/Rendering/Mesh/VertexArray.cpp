#include "VertexArray.h"

#include "../Renderer.h"
#include "GraphicAPI/Vulkan/Rendering/Mesh/VulkanVertexArray.h"

namespace Vectrix {
    std::shared_ptr<VertexArray> VertexArray::create() {
        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
            case RendererAPI::API::Vulkan:  return std::make_shared<VulkanVertexArray>();
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
        return nullptr;
    }
} // Vectrix