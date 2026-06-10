#ifndef VECTRIXWORKSPACE_ASSETSMANAGER_H
#define VECTRIXWORKSPACE_ASSETSMANAGER_H

#include <memory>
#include <filesystem>
#include <string>

#include "Vectrix/Rendering/Mesh/Mesh.h"
#include "Vectrix/Rendering/Shaders/ShaderManager.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"
#include "Vectrix/Rendering/Mesh/MeshManager.h"
#include "Vectrix/Utils/Memory.h"

namespace Vectrix {

    enum AssetType {
        UNKNOWN,
        TEXTURE,
        SHADER,
        MESH,
        SCENE
    };

    enum ResultAssetLoading {
        SUCCESS,
        UNKNOWN_TYPE,
        NOT_FOUND,
        WRONG_TYPE
    };
    inline std::string toString(ResultAssetLoading r) {
        switch (r) {
            case SUCCESS: return "SUCCESS";
            case UNKNOWN_TYPE: return "UNKNOWN_TYPE";
            case NOT_FOUND: return "NOT_FOUND";
            case WRONG_TYPE: return "WRONG_TYPE";
        }
        return "UNKNOWN_RESULT_ASSET_LOADING";
    }

    class AssetsManager {
    public:
        AssetsManager();
        ~AssetsManager();

        /**
         * This function loads an asset and return an error message if it has any and the ptr to the assets
         * @tparam T The class of the asset you wanna get
         * @param path The path of the assets
         */
        template<typename T>
        static std::pair<ResultAssetLoading, std::shared_ptr<T>> load(const std::string& path);



        static AssetsManager& instance() { return *s_instance; }
        TextureManager& getTextureManager() const { return *m_textureManager; }
        ShaderManager& getShaderManager() const { return *m_shaderManager; }
        MeshManager& getMeshManager() const { return *m_meshManager; }
    private:
        friend class TextureManager;
        friend class ShaderManager;
        static AssetType getAssetType(const std::filesystem::path &path);
        Cache<std::string, std::shared_ptr<void>> m_cache;
        std::unique_ptr<TextureManager> m_textureManager;
        std::unique_ptr<ShaderManager> m_shaderManager;
        std::unique_ptr<MeshManager> m_meshManager;


        static AssetsManager* s_instance;
    };

    template<>
    inline std::pair<ResultAssetLoading, std::shared_ptr<Texture>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);
        if (p.empty())
            return {ResultAssetLoading::NOT_FOUND, nullptr};

        if (getAssetType(path)!=TEXTURE) {
            return {ResultAssetLoading::WRONG_TYPE,nullptr};
        }

        auto it = s_instance->m_cache.find(p.c_str());
        if (it != s_instance->m_cache.end())
            return {ResultAssetLoading::SUCCESS, std::static_pointer_cast<Texture>(it->second)};

        auto texture = s_instance->m_textureManager->createTexture(p.c_str(), path);
        s_instance->m_cache.emplace(p.c_str(), texture);
        return {ResultAssetLoading::SUCCESS, texture};
    }

    template<>
    inline std::pair<ResultAssetLoading, std::shared_ptr<Shader>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);
        if (p.empty())
            return {ResultAssetLoading::NOT_FOUND, nullptr};

        if (getAssetType(path)!=SHADER) {
            return {ResultAssetLoading::WRONG_TYPE,nullptr};
        }

        auto it = s_instance->m_cache.find(p.c_str());
        if (it != s_instance->m_cache.end())
            return {ResultAssetLoading::SUCCESS, std::static_pointer_cast<Shader>(it->second)};

        auto shader = s_instance->m_shaderManager->createShader(p.c_str(), path);
        s_instance->m_cache.emplace(p.c_str(), shader);
        return {ResultAssetLoading::SUCCESS, shader};
    }

    template<>
    inline std::pair<ResultAssetLoading, std::shared_ptr<Mesh>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);
        if (p.empty())
            return {ResultAssetLoading::NOT_FOUND, nullptr};

        if (getAssetType(path)!=MESH) {
            return {ResultAssetLoading::WRONG_TYPE,nullptr};
        }

        auto it = s_instance->m_cache.find(p.c_str());
        if (it != s_instance->m_cache.end())
            return {ResultAssetLoading::SUCCESS, std::static_pointer_cast<Mesh>(it->second)};

        auto shader = s_instance->m_meshManager->createMesh(p.c_str(), path);
        s_instance->m_cache.emplace(p.c_str(), shader);
        return {ResultAssetLoading::SUCCESS, shader};
    }
} // Vectrix

#endif //VECTRIXWORKSPACE_ASSETSMANAGER_H