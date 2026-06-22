#include "AssetsManager.h"

#include "Vectrix/Rendering/Textures/Texture.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"

namespace Vectrix {
    AssetsManager* AssetsManager::s_instance = nullptr;
    std::filesystem::path AssetsManager::s_assetsPath = "./assets";
    static const std::unordered_map<std::string, AssetType> EXTENSION_MAP = {
        {".png",  TEXTURE},
        {".jpg",  TEXTURE},
        {".obj",  MESH},
        {".vcshader", SHADER},
        {".vctx", SCENE},
    };


    AssetsManager::AssetsManager() {
        VC_CORE_ASSERT(!s_instance, "AssetsManager already exists");
        s_instance = this;

        auto* s = new ShaderManager();
        m_shaderManager = std::unique_ptr<ShaderManager>(s);

        auto* t = new TextureManager();
        m_textureManager = std::unique_ptr<TextureManager>(t);

        auto* m = new MeshManager();
        m_meshManager = std::unique_ptr<MeshManager>(m);
    }

    AssetsManager::~AssetsManager() {
        m_shaderManager.reset();
        m_textureManager.reset();
        m_meshManager.reset();
        m_cache.clear();
    }


    AssetType AssetsManager::getAssetType(const std::filesystem::path &id) {
        auto it = EXTENSION_MAP.find(id.extension().string());
        if (it != EXTENSION_MAP.end())
            return it->second;
        return UNKNOWN;
    }
} // Vectrix