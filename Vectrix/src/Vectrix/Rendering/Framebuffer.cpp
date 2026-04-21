#include "Framebuffer.h"

#include "Renderer.h"
#include "GraphicAPI/Vulkan/Rendering/VulkanFramebuffer.h"

namespace Vectrix {
    Framebuffer* Framebuffer::s_currentFramebuffer = nullptr;

    std::shared_ptr<Framebuffer> Framebuffer::create(const FramebufferSpecification& spec)
    {
        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported"); return nullptr;
            case RendererAPI::API::Vulkan:  return std::make_shared<VulkanFramebuffer>(spec);
        }

        VC_CORE_ERROR("Unknown RendererAPI");
        return nullptr;
    }
} // Vectrix