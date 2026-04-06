#include "MeshManager.h"

#include <utility>

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
    MeshManager* MeshManager::s_instance = nullptr;


    MeshManager::MeshManager() {
        VC_CORE_ASSERT(!s_instance, "MeshManager already exists!");
        VC_CORE_INFO("Initializing MeshManager");
        s_instance = this;
    }

    MeshManager::~MeshManager() {
        VC_PROFILER_FUNCTION();
        VC_CORE_INFO("Destroying MeshManager");
        m_notFoundMesh.reset();
        m_cache.clear();
    }

    std::shared_ptr<Model> MeshManager::loadModel(const std::string &name, const std::string &path) {
        VC_PROFILER_FUNCTION();
        std::shared_ptr<Model> model = std::make_unique<Model>(Model::load(path));
        instance().add(name,model);
        Application::instance().window().getGraphicContext().registerMesh(name,model);
        return model;
    }

    std::shared_ptr<Model> MeshManager::createModel(const std::string &name, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const BufferLayout &layout) {
        VC_PROFILER_FUNCTION();
        std::shared_ptr<Model> model = std::make_unique<Model>(Model::create(vertices,indices,layout));
        instance().add(name,model);
        Application::instance().window().getGraphicContext().registerMesh(name,model);
        return model;
    }
    std::shared_ptr<Model> MeshManager::createModel(const std::string &name, const std::vector<Vertex> &vertices, const BufferLayout &layout) {
        VC_PROFILER_FUNCTION();
        std::shared_ptr<Model> model = std::make_unique<Model>(Model::create(vertices,layout));
        instance().add(name,model);
        Application::instance().window().getGraphicContext().registerMesh(name,model);
        return model;
    }

    void MeshManager::add(const std::string& name, std::shared_ptr<Model> model) {
        m_cache.emplace(name, model);
        VC_CORE_INFO("Model \"{}\" registered",name);
    }

    std::shared_ptr<Model> MeshManager::get(const std::string& name) {
        const auto it = m_cache.find(name);
        if (it == m_cache.end()) {
            VC_CORE_ERROR("Model with the name \"{}\" doesn't exist", name);
        }
        return it->second;
    }

    bool MeshManager::remove(const std::string& name) {
        return m_cache.erase(name);
    }
} // Vectrix