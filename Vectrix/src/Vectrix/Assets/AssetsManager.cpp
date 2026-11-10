#include "AssetsManager.h"

#include "Vectrix/Rendering/Textures/Texture.h"
#include "Vectrix/Rendering/Textures/TextureManager.h"
#include "Vectrix/Utils/Hashing.h"

#include <fstream>
#include "Vectrix/Core/CommandLine.h"
#include "Vectrix/Utils/Path.h"
#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Rendering/Mesh/ObjLoader.h"
#include "Vectrix/Rendering/Shaders/Shader.h"
#include "Vectrix/Threading/BackgroundWorker.h"
#include "Vectrix/Threading/JobSystem.h"
#include "Vectrix/Threading/Thread.h"

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


    // The engine's own assets: next to the executable in a package (in Contents/Resources inside a macOS bundle,
    // the executable being in Contents/MacOS), else in the working directory, which is how a build is run during
    // development (from run/, the executable staying in the build tree)
    static std::filesystem::path findEngineAssetsPath() {
        std::vector<std::filesystem::path> candidates;
        if (const std::filesystem::path executable = getExecutablePath(); !executable.empty()) {
            candidates.push_back(executable.parent_path() / "assets");
            candidates.push_back(executable.parent_path().parent_path() / "Resources" / "assets");
        }
        std::error_code ec;
        const std::filesystem::path workingDirectory = std::filesystem::current_path(ec);
        if (!ec)
            candidates.push_back(workingDirectory / "assets");

        for (const std::filesystem::path& candidate : candidates) {
            if (std::filesystem::is_directory(candidate, ec)) {
                VC_CORE_INFO("Engine assets: {}", toUtf8(candidate));
                return candidate;
            }
        }
        VC_CORE_WARN("Engine assets not found next to the executable nor in the working directory ({})", toUtf8(workingDirectory));
        return "./assets";
    }

    AssetsManager::AssetsManager() {
        VC_CORE_ASSERT(!s_instance, "AssetsManager already exists");
        s_instance = this;

        s_engineAssetsPath = findEngineAssetsPath();
        // Without a project the relative paths are taken from the engine's assets too (see getAssetsPath)
        if (s_assetsPath == "./assets")
            s_assetsPath = s_engineAssetsPath;

        auto* s = new ShaderManager();
        m_shaderManager = std::unique_ptr<ShaderManager>(s);

        auto* t = new TextureManager();
        m_textureManager = std::unique_ptr<TextureManager>(t);

        auto* m = new MeshManager();
        m_meshManager = std::unique_ptr<MeshManager>(m);
    }

    AssetsManager::~AssetsManager() {
        // The Application waited for the jobs (JobSystem::waitIdle): what they finished is dropped
        m_finished.clear();
        m_loading.clear();
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
        if (!sameFolder && s_instance) {
            s_instance->forgetProjectAssets();
            // A load still running for the previous folder delivers its asset but doesn't register it
            ++s_instance->m_assetsFolderGeneration;
        }
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
namespace Vectrix {
    std::shared_ptr<detail::AssetLoadState> AssetsManager::startLoad(const std::string& path, const AssetType type) {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT_MAIN_THREAD();
        auto state = std::make_shared<detail::AssetLoadState>();
        const auto [file, id, inAssetsFolder] = resolve(path);
        state->type = type;
        state->id = id;
        state->cacheKey = toGenericUtf8(file);
        state->inAssetsFolder = inAssetsFolder;
        state->assetsFolderGeneration = m_assetsFolderGeneration;

        const auto finished = [&state](const VectrixResult result, std::shared_ptr<void> asset) {
            state->done = true;
            state->result = result;
            state->asset = std::move(asset);
            return state;
        };
        if (!std::filesystem::exists(file))
            return finished(NOT_FOUND, nullptr);
        if (getAssetType(file) != type)
            return finished(WRONG_TYPE, nullptr);

        if (const auto it = m_cache.find(state->cacheKey); it != m_cache.end()) {
            // Registered under its id again if the type's manager was cleared, as load does
            switch (type) {
                case TEXTURE: if (!m_textureManager->m_cache.contains(id)) m_textureManager->add(id, std::static_pointer_cast<Texture>(it->second)); break;
                case SHADER: if (!m_shaderManager->exist(id)) m_shaderManager->add(id, std::static_pointer_cast<Shader>(it->second)); break;
                case MESH: if (!m_meshManager->exist(id)) m_meshManager->add(id, std::static_pointer_cast<Mesh>(it->second)); break;
                default: break;
            }
            return finished(SUCCESS, it->second);
        }
        if (const auto it = m_loading.find(state->cacheKey); it != m_loading.end())
            return it->second;

        m_loading.emplace(state->cacheKey, state);
        // The job only gets what it reads: the state's fields set above, and the file
        state->job = JobSystem::get().submit([this, state, file = toUtf8(file)] {
            VC_PROFILER_SCOPE("AssetsManager load job");
            std::function<std::pair<VectrixResult, std::shared_ptr<void>>()> finish;
            try {
                switch (state->type) {
                    case TEXTURE: {
                        // Decoded, uploaded and given its ImGui descriptor here: the device allows it on any thread
                        std::shared_ptr<Texture> texture = Texture::create(state->id, file);
                        finish = [texture] { return std::pair<VectrixResult, std::shared_ptr<void>>{texture ? SUCCESS : FORMATING_ERROR, texture}; };
                        break;
                    }
                    case SHADER: {
                        // Compiled and its pipeline built here
                        std::shared_ptr<Shader> shader = Shader::create(state->id, file, getTinyObjLayout());
                        finish = [shader] { return std::pair<VectrixResult, std::shared_ptr<void>>{shader ? SUCCESS : FORMATING_ERROR, shader}; };
                        break;
                    }
                    case MESH: {
                        // Only parsed here: its buffers come from the MeshRegistry, which is the main thread's
                        auto vertices = std::make_shared<std::vector<Vertex>>();
                        auto indices = std::make_shared<std::vector<uint32_t>>();
                        const bool parsed = loadOBJ(file, *vertices, *indices);
                        finish = [parsed, vertices, indices, id = state->id]() -> std::pair<VectrixResult, std::shared_ptr<void>> {
                            if (!parsed)
                                return {FORMATING_ERROR, nullptr};
                            auto mesh = std::make_shared<Mesh>(*vertices, *indices);
                            mesh->m_id = id;
                            return {SUCCESS, mesh};
                        };
                        break;
                    }
                    default:
                        finish = [] { return std::pair<VectrixResult, std::shared_ptr<void>>{UNKNOWN_TYPE, nullptr}; };
                }
            } catch (const std::exception& exception) {
                VC_CORE_ERROR_NO_EXIT("Loading {} failed: {}", state->id, exception.what());
                finish = [] { return std::pair<VectrixResult, std::shared_ptr<void>>{UNKNOWN_ERROR, nullptr}; };
            }
            state->finish = std::move(finish);
            {
                const std::scoped_lock lock(m_finishedMutex);
                m_finished.push_back(state);
            }
            detail::wakeMainLoop();
        });
        return state;
    }

    void AssetsManager::processLoadedAssets() {
        VC_PROFILER_FUNCTION();
        std::vector<std::shared_ptr<detail::AssetLoadState>> finished;
        {
            const std::scoped_lock lock(m_finishedMutex);
            finished.swap(m_finished);
        }
        for (const std::shared_ptr<detail::AssetLoadState>& state : finished)
            finishLoad(state);
    }

    void AssetsManager::finishLoad(const std::shared_ptr<detail::AssetLoadState>& state) {
        if (state->done)
            return;
        auto [result, asset] = state->finish();
        state->finish = nullptr;
        if (const auto it = m_loading.find(state->cacheKey); it != m_loading.end() && it->second == state)
            m_loading.erase(it);

        // Registered as load registers it, unless the assets folder changed meanwhile: its id meant the previous
        // folder's file. A failure isn't cached, so a fixed file can be loaded again
        if (result == SUCCESS && state->assetsFolderGeneration == m_assetsFolderGeneration && !m_cache.contains(state->cacheKey)) {
            m_cache.emplace(state->cacheKey, asset);
            switch (static_cast<AssetType>(state->type)) {
                case TEXTURE: m_textureManager->add(state->id, std::static_pointer_cast<Texture>(asset)); break;
                case SHADER: if (!m_shaderManager->exist(state->id)) m_shaderManager->add(state->id, std::static_pointer_cast<Shader>(asset)); break;
                case MESH: if (!m_meshManager->exist(state->id)) m_meshManager->add(state->id, std::static_pointer_cast<Mesh>(asset)); break;
                default: break;
            }
            if (state->inAssetsFolder)
                m_projectAssets.push_back({static_cast<AssetType>(state->type), state->id, state->cacheKey});
        }

        state->done = true;
        state->result = result;
        state->asset = std::move(asset);
        // Moved out first: a callback may start other loads, or wait for one
        std::vector<std::function<void()>> callbacks = std::move(state->callbacks);
        state->callbacks.clear();
        for (std::function<void()>& callback : callbacks)
            callback();
    }

    void AssetsManager::waitForRunningLoad(const std::string& cacheKey) {
        if (!s_instance)
            return;
        const auto it = s_instance->m_loading.find(cacheKey);
        if (it == s_instance->m_loading.end())
            return;
        const std::shared_ptr<detail::AssetLoadState> state = it->second; // the map entry goes when it finishes
        detail::waitForAssetLoad(state);
    }

    namespace detail {
        void waitForAssetLoad(const std::shared_ptr<AssetLoadState>& state) {
            VC_PROFILER_FUNCTION();
            VC_CORE_ASSERT_MAIN_THREAD();
            if (state->done)
                return;
            // Runs queued jobs meanwhile, this one included if no worker took it yet
            JobSystem::get().wait(state->job);
            AssetsManager::instance().processLoadedAssets();
            VC_CORE_ASSERT(state->done, "Asset load {} not finished after its job", state->id);
        }
    }
}
