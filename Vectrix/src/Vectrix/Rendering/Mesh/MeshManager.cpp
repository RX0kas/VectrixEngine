#include "MeshManager.h"

#include <memory>

#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
    MeshManager::MeshManager() {
        VC_CORE_INFO("Initializing MeshManager");
    }

    bool MeshManager::remove(const std::string &name) {
        return m_cache.erase(name);
    }

    std::shared_ptr<Mesh> MeshManager::createMesh(const std::string &name, const std::string &path) {
        VC_PROFILER_FUNCTION();
        VC_CORE_ASSERT(!exist(name),"A mesh with the name {} already exist",name);
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        if (!loadOBJ(path, vertices, indices)) {
            VC_CORE_CRITICAL("Failed to load OBJ");
        }
        auto mesh = std::make_shared<Mesh>(vertices,indices);
        mesh->m_id = name;
        add(name,mesh);

        return mesh;
    }

    MeshManager::~MeshManager() {
        VC_CORE_INFO("Destroying MeshManager");
        m_cache.clear();
    }

    void MeshManager::add(const std::string& name, std::shared_ptr<Mesh> mesh) {
        m_cache.emplace(name, std::move(mesh));
        VC_CORE_INFO("Mesh \"{}\" registered",name);
    }

    std::shared_ptr<Mesh> MeshManager::get(const std::string& name) {
        const auto it = m_cache.find(name);
        if (it == m_cache.end()) {
            VC_CORE_ERROR("Mesh with the name \"{}\" doesn't exist", name);
        }
        return it->second;

    }
} // Vectrix