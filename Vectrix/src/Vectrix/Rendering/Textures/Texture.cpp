#include "Texture.h"

#include <filesystem>

#include "TextureManager.h"
#include "GraphicAPI/Vulkan/Rendering/VulkanTexture.h"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Rendering/Renderer.h"
#include "Vectrix/Rendering/RendererAPI.h"

namespace Vectrix {
    unsigned int Texture::m_count = 0;

    std::shared_ptr<Texture> Texture::create(const std::string &path) {
        if (m_count==getMaxTexturePerShader()) {
            VC_CORE_ERROR("The maximum number of texture allowed is {}",getMaxTexturePerShader());
        }

        if (!std::filesystem::exists(path)) {
            VC_CORE_WARN("Texture {} doesn't exist, using default Texture instead",path);
            return TextureManager::getNotFoundTexture();
        }

        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported");
            case RendererAPI::API::Vulkan:  return std::make_unique<VulkanTexture>(path);
        }

        VC_CORE_ERROR("Unknown RendererAPI");
        return nullptr;
    }

    std::shared_ptr<Texture> Texture::createDefaultTexture() {
        switch (Renderer::getAPI())
        {
            case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported");
            case RendererAPI::API::Vulkan:  const auto t = new VulkanTexture();return std::shared_ptr<VulkanTexture>(t);
        }

        VC_CORE_ERROR("Unknown RendererAPI");
        return nullptr;
    }
} // Vectrix