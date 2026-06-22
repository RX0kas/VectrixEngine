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
        static std::string getAssetsPath() {
            return s_assetsPath.string();
        }

    private:
        friend class TextureManager;
        friend class ShaderManager;
        static AssetType getAssetType(const std::filesystem::path &path);
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
        if (p.empty())
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(path)!=TEXTURE) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }
        if (p.is_relative()) {
            std::filesystem::path tempPath;
            tempPath.append(s_assetsPath.string());
            tempPath.append(p.string());
            p = tempPath;
        }

        auto it = s_instance->m_cache.find(p.c_str());
        if (it != s_instance->m_cache.end())
            return {VectrixResult::SUCCESS, std::static_pointer_cast<Texture>(it->second)};

        auto texture = s_instance->m_textureManager->createTexture(path, p.string());
        s_instance->m_cache.emplace(path, texture);
        return {VectrixResult::SUCCESS, texture};
    }

    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Shader>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);
        if (p.empty())
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(path)!=SHADER) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        if (p.is_relative()) {
            std::filesystem::path tempPath;
            tempPath.append(s_assetsPath.string());
            tempPath.append(p.string());
            p = tempPath;
        }

        auto it = s_instance->m_cache.find(p.c_str());
        if (it != s_instance->m_cache.end())
            return {VectrixResult::SUCCESS, std::static_pointer_cast<Shader>(it->second)};

        auto shader = s_instance->m_shaderManager->createShader(path, p.string());
        s_instance->m_cache.emplace(path, shader);
        return {VectrixResult::SUCCESS, shader};
    }

    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Mesh>> AssetsManager::load(const std::string& path) {
        std::filesystem::path p(path);
        if (p.empty())
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(path)!=MESH) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        if (p.is_relative()) {
            std::filesystem::path tempPath;
            tempPath.append(s_assetsPath.string());
            tempPath.append(p.string());
            p = tempPath;
            VC_CORE_INFO(p.c_str());
        }

        auto it = s_instance->m_cache.find(p.c_str());
        if (it != s_instance->m_cache.end())
            return {VectrixResult::SUCCESS, std::static_pointer_cast<Mesh>(it->second)};

        auto mesh = s_instance->m_meshManager->createMesh(path, p.string());
        s_instance->m_cache.emplace(path, mesh);
        return {VectrixResult::SUCCESS, mesh};
    }
} // Vectrix

#endif //VECTRIXWORKSPACE_ASSETSMANAGER_H