#include "Texture.h"

#include <filesystem>

#include "TextureManager.h"
#include "GraphicAPI/Vulkan/Rendering/VulkanTexture.h"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Rendering/Renderer.h"
#include "Vectrix/Rendering/RendererAPI.h"

namespace Vectrix {
    unsigned int Texture::m_count = 0;

    Ref<Texture> Texture::create(const std::string &path) {
        if (m_count==getMaxTexture()) {
            VC_CORE_ERROR("The maximum number of texture allowed is {}",getMaxTexture());
        }

        if (!std::filesystem::exists(path)) {
            VC_CORE_WARN("Texture {} doesn't exist, using default Texture instead",path);
            return TextureManager::getNotFoundTexture();
        }

        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported");
            case RendererAPI::API::Vulkan:  return std::make_shared<VulkanTexture>(path);
        }

        VC_CORE_ERROR("Unknown RendererAPI");
    }

    Ref<Texture> Texture::createDefaultTexture() {
        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported");
            case RendererAPI::API::Vulkan:  auto t = new VulkanTexture();return std::shared_ptr<VulkanTexture>(t);
        }

        VC_CORE_ERROR("Unknown RendererAPI");
    }
} // Vectrix