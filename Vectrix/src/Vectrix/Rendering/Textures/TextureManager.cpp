#include "Vectrix/Rendering/Textures/TextureManager.h"

#include <utility>

#include "Vectrix/Core/Log.h"

namespace Vectrix {
    TextureManager* TextureManager::p_instance = nullptr;


    TextureManager::TextureManager() {
        VC_CORE_INFO("Initializing TextureManager");
        VC_CORE_ASSERT(!p_instance, "TextureManager already exists!");
        p_instance = this;
        p_cache.reserve(256);
        p_cache.max_load_factor(0.7f);

        m_notFoundTexture = Texture::create("textures/not_found.png"); // TODO: move it into the executable
    }

    TextureManager::~TextureManager() {
        p_cache.clear();
    }

    void TextureManager::createTexture(const std::string &name, const std::string &path) {
        Ref<Texture> texture(Texture::create(path));
        instance().add(name,std::move(texture));
    }

    void TextureManager::add(const std::string& name, Ref<Texture> texture) {
        p_cache.emplace(name, std::move(texture));
        VC_CORE_INFO("Texture \"{}\" registered",name);
    }

    Ref<Texture> TextureManager::get(const std::string& name) {
        auto it = p_cache.find(name);
        if (it == p_cache.end()) {
            VC_CORE_ERROR("Texture with the name \"{}\" doesn't exist", name);
        }
        return it->second;
    }

    Ref<Texture> TextureManager::getNotFoundTexture() {
        return instance().m_notFoundTexture;
    }

    bool TextureManager::remove(const std::string& name) {
        return p_cache.erase(name);
    }
} // Vectrix