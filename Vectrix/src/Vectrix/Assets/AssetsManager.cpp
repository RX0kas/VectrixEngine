#include "AssetsManager.h"

#include "Vectrix/Rendering/Textures/Texture.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"
#include "Vectrix/Utils/Hashing.h"

#include <fstream>
#include "Vectrix/Utils/Path.h"

namespace Vectrix {
    AssetsManager* AssetsManager::s_instance = nullptr;
    std::filesystem::path AssetsManager::s_assetsPath = "./assets";
    std::filesystem::path AssetsManager::s_engineAssetsPath = "./assets";
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

    AssetsManager::ResolvedAsset AssetsManager::resolve(const std::string& path) {
        // absolute(): the assets folder itself may be relative (a project path given on the command line)
        std::error_code ec;
        const std::filesystem::path root = std::filesystem::absolute(s_assetsPath, ec).lexically_normal();
        const std::filesystem::path requested = fromUtf8(path);
        const std::filesystem::path file = std::filesystem::absolute(requested.is_relative() ? s_assetsPath / requested : requested, ec).lexically_normal();

        // Empty for another drive on Windows, starts with ".." for anything outside the folder
        const std::filesystem::path relative = file.lexically_relative(root);
        const bool insideAssets = !relative.empty() && *relative.begin() != "..";
        return {file, insideAssets ? toGenericUtf8(relative) : toGenericUtf8(file), insideAssets};
    }

    void AssetsManager::setAssetsPath(const std::filesystem::path& path) {
        std::error_code ec;
        const bool sameFolder = std::filesystem::absolute(path, ec).lexically_normal() == std::filesystem::absolute(s_assetsPath, ec).lexically_normal();
        s_assetsPath = path;
        // Ids are relative to the folder: another project's "shaders/a.vcshader" would otherwise be taken for
        // (or collide with) the previous project's
        if (!sameFolder && s_instance)
            s_instance->forgetProjectAssets();
    }

    std::optional<std::string> AssetsManager::getProjectAssetId(const std::filesystem::path& path) {
        ResolvedAsset asset = resolve(toUtf8(path));
        if (!asset.inAssetsFolder)
            return std::nullopt;
        return std::move(asset.id);
    }

    std::optional<std::string> AssetsManager::remapAssetId(const std::string& id, const std::string& oldId, const std::string& newId) {
        if (id == oldId)
            return newId;
        // Inside a folder that moved: whole path components only ("tex" doesn't hold "textures/a.png")
        if (id.size() > oldId.size() && id.starts_with(oldId) && id[oldId.size()] == '/')
            return newId + id.substr(oldId.size());
        return std::nullopt;
    }

    void AssetsManager::moveProjectAssets(const std::string& oldId, const std::string& newId) {
        for (ProjectAsset& asset : m_projectAssets) {
            std::optional<std::string> movedId = remapAssetId(asset.id, oldId, newId);
            if (!movedId)
                continue;

            std::shared_ptr<void> object;
            if (const auto it = m_cache.find(asset.cacheKey); it != m_cache.end()) {
                object = it->second;
                m_cache.erase(it);
            }
            std::string cacheKey = toGenericUtf8(resolve(*movedId).file);
            if (object)
                m_cache.emplace(cacheKey, object);

            // The type managers may not hold it anymore (EditorLayer::openScene clears the texture and mesh ones)
            switch (object ? asset.type : UNKNOWN) {
                case TEXTURE: {
                    const auto texture = std::static_pointer_cast<Texture>(object);
                    texture->setID(*movedId);
                    m_textureManager->remove(asset.id);
                    m_textureManager->add(*movedId, texture);
                    break;
                }
                case SHADER: {
                    const auto shader = std::static_pointer_cast<Shader>(object);
                    shader->setID(*movedId);
                    m_shaderManager->remove(asset.id);
                    m_shaderManager->add(*movedId, shader);
                    break;
                }
                case MESH: {
                    const auto mesh = std::static_pointer_cast<Mesh>(object);
                    mesh->m_id = *movedId;
                    m_meshManager->remove(asset.id);
                    m_meshManager->add(*movedId, mesh);
                    break;
                }
                default: break;
            }
            VC_CORE_INFO("Asset {} moved to {}", asset.id, *movedId);
            asset.id = std::move(*movedId);
            asset.cacheKey = std::move(cacheKey);
        }
    }

    std::optional<AssetFingerprint> AssetsManager::getFingerprint(const std::string& id) {
        const std::filesystem::path file = resolve(id).file;
        std::error_code ec;
        const std::uint64_t size = std::filesystem::file_size(file, ec);
        if (ec)
            return std::nullopt;
        const std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(file, ec);
        if (ec)
            return std::nullopt;

        const std::string key = toGenericUtf8(file);
        if (const auto it = s_instance->m_fingerprints.find(key); it != s_instance->m_fingerprints.end()
            && it->second.size == size && it->second.writeTime == writeTime)
            return AssetFingerprint{size, it->second.hash};

        std::ifstream stream(file, std::ios::binary);
        if (!stream)
            return std::nullopt;
        XXH3_state_t* state = XXH3_createState();
        XXH3_64bits_reset_withSeed(state, XXH3::seed);
        std::vector<char> buffer(1 << 16);
        while (stream) {
            stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            if (stream.gcount() > 0)
                XXH3_64bits_update(state, buffer.data(), static_cast<size_t>(stream.gcount()));
        }
        const std::uint64_t hash = XXH3_64bits_digest(state);
        XXH3_freeState(state);

        s_instance->m_fingerprints[key] = {size, writeTime, hash};
        return AssetFingerprint{size, hash};
    }

    std::optional<AssetsManager::MovedAsset> AssetsManager::findMovedAsset(const std::string& id, const AssetFingerprint& fingerprint) {
        const std::filesystem::path missing = fromUtf8(id);
        std::vector<std::filesystem::path> sameContent;
        std::vector<std::filesystem::path> sameName;

        std::error_code ec;
        for (auto it = std::filesystem::recursive_directory_iterator(s_assetsPath, std::filesystem::directory_options::skip_permission_denied, ec);
             !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
            if (!it->is_regular_file(ec) || it->path().extension() != missing.extension())
                continue;
            if (it->path().filename() == missing.filename())
                sameName.push_back(it->path());
            // Only files of the same size are hashed
            if (fingerprint.isKnown() && it->file_size(ec) == fingerprint.size) {
                if (const auto candidate = getFingerprint(toUtf8(it->path())); candidate && *candidate == fingerprint)
                    sameContent.push_back(it->path());
            }
        }

        std::optional<std::filesystem::path> found;
        bool byNameOnly = false;
        if (sameContent.size() == 1) {
            found = sameContent.front();
        } else if (sameContent.size() > 1) {
            // Identical copies: the one that kept its name, if only one did
            std::vector<std::filesystem::path> named;
            std::ranges::copy_if(sameContent, std::back_inserter(named), [&](const auto& path) { return path.filename() == missing.filename(); });
            if (named.size() == 1)
                found = named.front();
        } else if (sameName.size() == 1) {
            // No file with its content (it was edited as well as moved, or the scene was saved before fingerprints):
            // the one file with its name, which the caller has to present as a guess
            found = sameName.front();
            byNameOnly = true;
        }

        if (!found)
            return std::nullopt;
        std::optional<std::string> newId = getProjectAssetId(*found);
        if (!newId)
            return std::nullopt;
        return MovedAsset{std::move(*newId), byNameOnly};
    }

    void AssetsManager::forgetProjectAssets() {
        if (m_projectAssets.empty())
            return;
        for (const auto& [type, id, cacheKey] : m_projectAssets) {
            m_cache.erase(cacheKey);
            switch (type) {
                case TEXTURE: m_textureManager->remove(id); break;
                case SHADER: m_shaderManager->remove(id); break;
                case MESH: m_meshManager->remove(id); break;
                default: break;
            }
        }
        VC_CORE_INFO("Released the {} assets of the previous assets folder", m_projectAssets.size());
        m_projectAssets.clear();
    }


    AssetType AssetsManager::getAssetType(const std::filesystem::path &id) {
        auto it = EXTENSION_MAP.find(toUtf8(id.extension()));
        if (it != EXTENSION_MAP.end())
            return it->second;
        return UNKNOWN;
    }
} // Vectrix