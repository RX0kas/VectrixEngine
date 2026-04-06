#include "Vectrix/Rendering/Textures/TextureManager.h"

#include <utility>

#include "Vectrix/Core/Log.h"

namespace Vectrix {
    TextureManager* TextureManager::p_instance = nullptr;


    TextureManager::TextureManager() {
        VC_CORE_INFO("Initializing TextureManager");
        VC_CORE_ASSERT(!p_instance, "TextureManager already exists!");
        p_instance = this;
        m_notFoundTexture = Texture::createDefaultTexture();
    }

    std::shared_ptr<Texture> TextureManager::createTexture(const std::string &name, const std::string &path) {
        std::shared_ptr<Texture> texture(Texture::create(path));
        instance().add(name,texture);
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
        return instance().m_notFoundTexture;
    }

    bool TextureManager::remove(const std::string& name) {
        return m_cache.erase(name);
    }
} // Vectrix