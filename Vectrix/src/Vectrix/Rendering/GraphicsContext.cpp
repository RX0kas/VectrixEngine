#include "GraphicsContext.h"

#include "Renderer.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
    GraphicsContext *GraphicsContext::create(GLFWwindow *windowHandle) {
        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
            case RendererAPI::API::Vulkan:  return new VulkanContext(windowHandle);
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
    }

    void GraphicsContext::setClientAPI() {
        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");return;
            case RendererAPI::API::Vulkan:  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);return;
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
    }
}
