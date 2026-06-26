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
#include "Vectrix/Utils/Result.h"

#define VC_LOAD_ASSET(variable,type,path) {auto t = AssetsManager::load<type>(path);if (t.first!=SUCCESS) {VC_ERROR_NO_EXIT("Can't load asset from {}: {}",path,toString(t.first));} variable = t.second;}

namespace Vectrix {

    enum AssetType {
        UNKNOWN,
        TEXTURE,
        SHADER,
        MESH,
        SCENE
    };

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
        static std::pair<VectrixResult, std::shared_ptr<T>> load(const std::string& path);



        static AssetsManager& instance() { return *s_instance; }
        TextureManager& getTextureManager() const { return *m_textureManager; }
        ShaderManager& getShaderManager() const { return *m_shaderManager; }
        MeshManager& getMeshManager() const { return *m_meshManager; }
        static std::filesystem::path getAssetsPath() {
            return s_assetsPath;
        }

        static AssetType getAssetType(const std::filesystem::path &path);
    private:
        friend class TextureManager;
        friend class ShaderManager;
        Cache<std::string, std::shared_ptr<void>> m_cache;
        std::unique_ptr<TextureManager> m_textureManager;
        std::unique_ptr<ShaderManager> m_shaderManager;
        std::unique_ptr<MeshManager> m_meshManager;
        static std::filesystem::path s_assetsPath;

        static AssetsManager* s_instance;
    };

    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Texture>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);

        if (p.is_relative()) {
            std::filesystem::path tempPath;
            tempPath.append(s_assetsPath.string());
            tempPath.append(p.string());
            p = tempPath;
        }
        // No need to check if the texture exist because the default texture will be used

        if (getAssetType(path)!=TEXTURE) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        const std::string cacheKey = p.lexically_normal().generic_string();
        auto it = s_instance->m_cache.find(cacheKey);
        if (it != s_instance->m_cache.end()) {
            auto texture = std::static_pointer_cast<Texture>(it->second);
            if (s_instance->m_textureManager->m_cache.find(path) == s_instance->m_textureManager->m_cache.end())
                s_instance->m_textureManager->add(path, texture);
            return {VectrixResult::SUCCESS, texture};
        }

        auto texture = s_instance->m_textureManager->createTexture(path, p.string());
        s_instance->m_cache.emplace(cacheKey, texture);
        return {VectrixResult::SUCCESS, texture};
    }

    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Shader>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);


        if (p.is_relative()) {
            std::filesystem::path tempPath;
            tempPath.append(s_assetsPath.string());
            tempPath.append(p.string());
            p = tempPath;
        }

        if (!std::filesystem::exists(p))
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(path)!=SHADER) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        const std::string cacheKey = p.lexically_normal().generic_string();
        auto it = s_instance->m_cache.find(cacheKey);
        if (it != s_instance->m_cache.end()) {
            auto shader = std::static_pointer_cast<Shader>(it->second);
            if (!s_instance->m_shaderManager->exist(path))
                s_instance->m_shaderManager->add(path, shader);
            return {VectrixResult::SUCCESS, shader};
        }

        auto shader = s_instance->m_shaderManager->createShader(path, p.string());
        s_instance->m_cache.emplace(cacheKey, shader);
        return {VectrixResult::SUCCESS, shader};
    }

    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Mesh>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);

        if (p.is_relative()) {
            std::filesystem::path tempPath;
            tempPath.append(s_assetsPath.string());
            tempPath.append(p.string());
            p = tempPath;
        }

        if (!std::filesystem::exists(p))
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(path)!=MESH) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        const std::string cacheKey = p.lexically_normal().generic_string();
        auto it = s_instance->m_cache.find(cacheKey);
        if (it != s_instance->m_cache.end()) {
            auto mesh = std::static_pointer_cast<Mesh>(it->second);
            if (!s_instance->m_meshManager->exist(path))
                s_instance->m_meshManager->add(path, mesh);
            return {VectrixResult::SUCCESS, mesh};
        }

        auto mesh = s_instance->m_meshManager->createMesh(path, p.string());
        s_instance->m_cache.emplace(cacheKey, mesh);
        return {VectrixResult::SUCCESS, mesh};
    }
} // Vectrix

#endif //VECTRIXWORKSPACE_ASSETSMANAGER_H
