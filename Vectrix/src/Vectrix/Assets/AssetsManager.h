#ifndef VECTRIXWORKSPACE_ASSETSMANAGER_H
#define VECTRIXWORKSPACE_ASSETSMANAGER_H

#include <memory>
#include <optional>
#include <filesystem>
#include <string>

#include "Vectrix/Rendering/Mesh/Mesh.h"
#include "Vectrix/Rendering/Shaders/ShaderManager.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"
#include "Vectrix/Rendering/Mesh/MeshManager.h"
#include "Vectrix/Utils/Memory.h"
#include "Vectrix/Utils/Result.h"
#include "Vectrix/Utils/Path.h"
#include "Vectrix/Assets/AssetFingerprint.h"

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

        template<typename T>
        static std::pair<VectrixResult, std::shared_ptr<T>> load(const std::filesystem::path& path)
        {
            return load<T>(toUtf8(path));
        }



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
         * @brief Return the folder every relative project asset path is taken from
         *
         * Set to `<project>/Assets` once a project is open. Before that (e.g. while
         * StartupLayer is showing) it falls back to #getEngineAssetsPath.
         * @return The project's assets folder
         * @see setAssetsPath
         */
        static std::filesystem::path getAssetsPath() {
            return s_assetsPath;
        }

        /**
         * @brief Set the folder every relative project asset path is taken from
         * @param path The project's assets folder, e.g. `<project>/Assets`
         * @note Does not affect #getEngineAssetsPath, which stays fixed: editor/engine
         *       built-in assets (icons, the ImGui shader, ...) never live in a project
         * @note Moving to another folder forgets the assets loaded from the previous one, so a
         *       project whose assets have the same relative paths gets its own
         * @see getAssetsPath
         */
        static void setAssetsPath(const std::filesystem::path& path);

        /**
         * @brief Return the folder the engine's own built-in assets are taken from
         *
         * Used for assets that ship with the engine/editor itself (the ImGui shader,
         * content browser icons, ...), never for project content, so it is not affected
         * by #setAssetsPath.
         * @return The engine's assets folder, as an absolute path
         * @note Returned absolute so callers can join it with a sub-path and pass the
         *       result straight to #load: load()'s relative-path handling would
         *       otherwise re-root a relative engine path under the project's assets
         *       folder (see #setAssetsPath) instead of leaving it alone.
         */
        static std::filesystem::path getEngineAssetsPath() {
            return std::filesystem::absolute(s_engineAssetsPath);
        }

        /**
         * @brief Work out what kind of asset a file holds, from its extension
         * @param path The path of the file
         * @return The kind of asset, #UNKNOWN when the extension is not handled
         * @note The file is not opened, only its extension is looked at
         */
        static AssetType getAssetType(const std::filesystem::path &path);

        /**
         * @brief The id a file of the assets folder is loaded, and saved in scenes, under
         * @param path The file (or folder), absolute or relative to the assets folder
         * @return The id, or nullopt for a path outside the assets folder
         */
        static std::optional<std::string> getProjectAssetId(const std::filesystem::path& path);

        /**
         * @brief An asset id after the file, or a folder holding it, moved
         * @param id The id to update
         * @param oldId The id of the file or folder that moved
         * @param newId Its id after the move
         * @return The id to use now, or nullopt when id is neither oldId nor inside it
         */
        static std::optional<std::string> remapAssetId(const std::string& id, const std::string& oldId, const std::string& newId);

        /**
         * @brief Follow a file or folder of the assets folder that moved
         *
         * The assets already loaded from it keep their objects, now registered and identified by their new ids, so
         * what uses them (an open scene) saves the new paths.
         * @param oldId The id of the file or folder before the move
         * @param newId Its id after the move
         * @note Scene files that refer to them are updated separately, see SceneSerializer::remapAssetIds
         */
        void moveProjectAssets(const std::string& oldId, const std::string& newId);

        /**
         * @brief The fingerprint of an asset file (its size and a hash of its content)
         * @param id The asset's id, or the path of the file
         * @return The fingerprint, or nullopt when the file can't be read
         * @note Computed again only when the file changed (its size or write time), as hashing a big texture takes a
         *       few milliseconds
         */
        static std::optional<AssetFingerprint> getFingerprint(const std::string& id);

        /// An asset found again by findMovedAsset
        struct MovedAsset {
            std::string id; ///< Its new id
            /// Found by its file name, not its content: the content changed (or the scene has no fingerprint for it),
            /// so it may be another file with the same name
            bool byNameOnly;
        };

        /**
         * @brief Look for an asset that was moved or renamed outside the editor, in the assets folder
         * @param id The path it had
         * @param fingerprint Its content, as saved in the scene (unknown for scenes saved before fingerprints)
         * @return The one file with the same content (of identical copies, the one with the same name). When no file
         *         has that content, the one file with the same name, marked byNameOnly. nullopt when there is none, or
         *         several with nothing to tell them apart
         */
        static std::optional<MovedAsset> findMovedAsset(const std::string& id, const AssetFingerprint& fingerprint);
    private:
        /// Where a requested asset is on disk, and the id it is registered and saved in scenes under
        struct ResolvedAsset {
            std::filesystem::path file; ///< Absolute and normalised
            std::string id; ///< Relative to the assets folder with '/' separators, or the absolute file outside it
            bool inAssetsFolder; ///< A project asset, forgotten when the assets folder changes
        };

        /// An asset loaded from the assets folder, as registered in the caches
        struct ProjectAsset {
            AssetType type;
            std::string id; ///< Its name in the type's manager
            std::string cacheKey; ///< Its key in m_cache
        };

        /// Drop the project assets from m_cache and the type managers (what still uses them keeps them alive)
        void forgetProjectAssets();

        /**
         * @brief Resolve a path given to #load
         *
         * A file inside the assets folder always gets the same id, whichever way it was asked for
         * ("./models/a.obj", an absolute path, '\' separators on Windows): it isn't registered twice,
         * and a scene saved on one machine or OS finds it on another.
         */
        static ResolvedAsset resolve(const std::string& path);

        friend class TextureManager;
        friend class ShaderManager;
        Cache<std::string, std::shared_ptr<void>> m_cache;
        std::vector<ProjectAsset> m_projectAssets;

        /// A fingerprint and the version of the file it was computed for
        struct CachedFingerprint {
            std::uint64_t size;
            std::filesystem::file_time_type writeTime;
            std::uint64_t hash;
        };
        std::unordered_map<std::string, CachedFingerprint> m_fingerprints; ///< Keyed by the file's absolute path
        std::unique_ptr<TextureManager> m_textureManager;
        std::unique_ptr<ShaderManager> m_shaderManager;
        std::unique_ptr<MeshManager> m_meshManager;
        static std::filesystem::path s_assetsPath;
        static std::filesystem::path s_engineAssetsPath;

        static AssetsManager* s_instance;
    };

    /// @cond INTERNAL
    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Texture>> AssetsManager::load(const std::string& path) {
        const auto [p, id, inAssetsFolder] = resolve(path);

        // Reported rather than replaced by the not_found texture: a caller like the scene loader has to know, to
        // keep the reference (and it isn't cached, so the file loads once it's back)
        if (!std::filesystem::exists(p))
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(p)!=TEXTURE) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        const std::string cacheKey = toGenericUtf8(p);
        auto it = s_instance->m_cache.find(cacheKey);
        if (it != s_instance->m_cache.end()) {
            auto texture = std::static_pointer_cast<Texture>(it->second);
            if (s_instance->m_textureManager->m_cache.find(id) == s_instance->m_textureManager->m_cache.end())
                s_instance->m_textureManager->add(id, texture);
            return {VectrixResult::SUCCESS, texture};
        }

        auto texture = s_instance->m_textureManager->createTexture(id, toUtf8(p));
        s_instance->m_cache.emplace(cacheKey, texture);
        if (inAssetsFolder)
            s_instance->m_projectAssets.push_back({TEXTURE, id, cacheKey});
        return {VectrixResult::SUCCESS, texture};
    }

    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Shader>> AssetsManager::load(const std::string& path) {
        const auto [p, id, inAssetsFolder] = resolve(path);

        if (!std::filesystem::exists(p))
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(p)!=SHADER) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        const std::string cacheKey = toGenericUtf8(p);
        auto it = s_instance->m_cache.find(cacheKey);
        if (it != s_instance->m_cache.end()) {
            auto shader = std::static_pointer_cast<Shader>(it->second);
            if (!s_instance->m_shaderManager->exist(id))
                s_instance->m_shaderManager->add(id, shader);
            return {VectrixResult::SUCCESS, shader};
        }

        auto shader = s_instance->m_shaderManager->createShader(id, toUtf8(p));
        if (!shader)
            return {VectrixResult::FORMATING_ERROR, nullptr}; // Doesn't compile. Not cached, so a fixed file can be loaded again
        s_instance->m_cache.emplace(cacheKey, shader);
        if (inAssetsFolder)
            s_instance->m_projectAssets.push_back({SHADER, id, cacheKey});
        return {VectrixResult::SUCCESS, shader};
    }

    template<>
    inline std::pair<VectrixResult, std::shared_ptr<Mesh>> AssetsManager::load(const std::string& path) {
        const auto [p, id, inAssetsFolder] = resolve(path);

        if (!std::filesystem::exists(p))
            return {VectrixResult::NOT_FOUND, nullptr};

        if (getAssetType(p)!=MESH) {
            return {VectrixResult::WRONG_TYPE,nullptr};
        }

        const std::string cacheKey = toGenericUtf8(p);
        auto it = s_instance->m_cache.find(cacheKey);
        if (it != s_instance->m_cache.end()) {
            auto mesh = std::static_pointer_cast<Mesh>(it->second);
            if (!s_instance->m_meshManager->exist(id))
                s_instance->m_meshManager->add(id, mesh);
            return {VectrixResult::SUCCESS, mesh};
        }

        auto mesh = s_instance->m_meshManager->createMesh(id, toUtf8(p));
        if (!mesh)
            return {VectrixResult::FORMATING_ERROR, nullptr}; // Not cached, so a fixed file can be loaded again
        s_instance->m_cache.emplace(cacheKey, mesh);
        if (inAssetsFolder)
            s_instance->m_projectAssets.push_back({MESH, id, cacheKey});
        return {VectrixResult::SUCCESS, mesh};
    }
    /// @endcond
} // Vectrix

#endif //VECTRIXWORKSPACE_ASSETSMANAGER_H
