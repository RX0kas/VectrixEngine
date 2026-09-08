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

/**
 * @file AssetsManager.h
 * @brief Definition of the AssetsManager, the single entry point to load an asset
 * @ingroup assets
 */

/**
 * @brief Load an asset into a variable, logging the reason when it fails
 *
 * A shorthand around AssetsManager::load for the common case where the failure only has
 * to be reported. The variable is left untouched when the load does not work out.
 * @param variable The variable receiving the loaded asset
 * @param type The asset class to load, such as `Texture` or `Mesh`
 * @param path The path of the asset, relative to the assets folder
 * @see AssetsManager::load
 * @ingroup assets
 */
#define VC_LOAD_ASSET(variable,type,path) {auto t = AssetsManager::load<type>(path);if (t.first!=SUCCESS) {VC_ERROR_NO_EXIT("Can't load asset from {}: {}",path,toString(t.first));} variable = t.second;}

namespace Vectrix {

    /**
     * @brief The kind of asset a file holds, worked out from its extension
     * @see AssetsManager::getAssetType
     * @ingroup assets
     */
    enum AssetType {
        UNKNOWN, ///< The extension is not one the engine handles
        TEXTURE, ///< An image, loaded as a Texture
        SHADER,  ///< A `.vcshader` file, loaded as a Shader
        MESH,    ///< A model file, loaded as a Mesh
        SCENE    ///< A scene file, loaded through SceneSerializer
    };

    /**
     * @brief Loads the assets of the project, and hands back the ones already loaded
     *
     * It sits above the per type managers and keeps a cache keyed on the resolved path,
     * so asking twice for the same file gives back the same object rather than loading it
     * again. The Application creates it, reach it through instance.
     * @see TextureManager
     * @see ShaderManager
     * @see MeshManager
     * @ingroup assets
     */
    class AssetsManager {
    public:
        /**
         * @brief Build the manager and the per type managers under it
         * @note The Application does this, an application should not need to
         */
        AssetsManager();
        ~AssetsManager();

        /**
         * @brief This function loads an asset and return an error message if it has any and the ptr to the assets
         *
         * A relative path is taken from the assets folder. When the same path was loaded
         * before, the cached asset is returned instead of being loaded again.
         * @tparam T The class of the asset you wanna get
         * @param path The path of the assets
         * @return Whether it worked, and the asset when it did
         * @see VC_LOAD_ASSET
         * @see getAssetsPath
         */
        template<typename T>
        static std::pair<VectrixResult, std::shared_ptr<T>> load(const std::string& path);



        /**
         * @brief Return the manager of the application
         * @return The single instance, created by the Application
         */
        static AssetsManager& instance() { return *s_instance; }

        /**
         * @brief Return the manager dealing with the textures
         * @return The texture manager, owned by this one
         */
        TextureManager& getTextureManager() const { return *m_textureManager; }

        /**
         * @brief Return the manager dealing with the shaders
         * @return The shader manager, owned by this one
         */
        ShaderManager& getShaderManager() const { return *m_shaderManager; }

        /**
         * @brief Return the manager dealing with the meshes
         * @return The mesh manager, owned by this one
         */
        MeshManager& getMeshManager() const { return *m_meshManager; }

        /**
         * @brief Return the folder every relative asset path is taken from
         * @return The assets folder
         */
        static std::filesystem::path getAssetsPath() {
            return s_assetsPath;
        }

        /**
         * @brief Work out what kind of asset a file holds, from its extension
         * @param path The path of the file
         * @return The kind of asset, #UNKNOWN when the extension is not handled
         * @note The file is not opened, only its extension is looked at
         */
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

    /// @cond INTERNAL
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
    /// @endcond
} // Vectrix

#endif //VECTRIXWORKSPACE_ASSETSMANAGER_H
