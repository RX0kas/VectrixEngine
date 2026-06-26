#include "Vectrix/Rendering/Textures/TextureManager.h"

#include <utility>

#include "Vectrix/Assets/AssetsManager.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    TextureManager::TextureManager() {
        VC_CORE_INFO("Initializing TextureManager");
        m_notFoundTexture = Texture::createDefaultTexture();
    }

    std::shared_ptr<Texture> TextureManager::createTexture(const std::string &name, const std::string &path) {
        const auto it = m_cache.find(name);
        if (it != m_cache.end())
            return it->second;

        std::shared_ptr<Texture> texture(Texture::create(name,path));
        add(name,texture);
        return texture;
    }

    TextureManager::~TextureManager() {
        VC_CORE_INFO("Destroying TextureManager");
        m_notFoundTexture.reset();
        m_cache.clear();
    }

    void TextureManager::add(const std::string& name, std::shared_ptr<Texture> texture) {
        m_cache.emplace(name, std::move(texture));
        VC_CORE_INFO("Texture \"{}\" registered",name);
    }

    std::shared_ptr<Texture> TextureManager::get(const std::string& name) {
        const auto it = m_cache.find(name);
        if (it == m_cache.end()) {
            VC_CORE_ERROR("Texture with the name \"{}\" doesn't exist", name);
        }
        return it->second;
    }

    std::shared_ptr<Texture> TextureManager::getNotFoundTexture() {
        return AssetsManager::instance().m_textureManager->m_notFoundTexture;
    }

    bool TextureManager::remove(const std::string& name) {
        return m_cache.erase(name);
    }
} // Vectrix
