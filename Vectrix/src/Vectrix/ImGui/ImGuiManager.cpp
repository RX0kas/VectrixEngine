#include "ImGuiManager.h"

#include "GraphicAPI/Vulkan/ImGui/VulkanImGuiManager.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    ImGuiManager* ImGuiManager::create(Window& window) {
        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
            case RendererAPI::API::Vulkan:  return new VulkanImGuiManager(window);
        }

        VC_CORE_ERROR("Unknown RendererAPI!");
    }
} // Vectrix