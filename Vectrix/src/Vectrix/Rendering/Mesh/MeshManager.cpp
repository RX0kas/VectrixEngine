#include "MeshManager.h"

#include <utility>

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"
#include "Vectrix/Core/Log.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
    MeshManager* MeshManager::p_instance = nullptr;


    MeshManager::MeshManager() {
        VC_CORE_INFO("Initializing MeshManager");
        VC_CORE_ASSERT(!p_instance, "MeshManager already exists!");
        p_instance = this;
    }

    Ref<Model> MeshManager::loadModel(const std::string &name, const std::string &path) {
        VC_PROFILER_FUNCTION();
        Ref<Model> model = createRef<Model>(Model::load(path));
        instance().add(name,model);
        Application::instance().window().getGraphicContext().registerMesh(name,model);
        return model;
    }

    Ref<Model> MeshManager::createModel(const std::string &name, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const BufferLayout &layout) {
        VC_PROFILER_FUNCTION();
        Ref<Model> model = createRef<Model>(Model::create(vertices,indices,layout));
        instance().add(name,model);
        Application::instance().window().getGraphicContext().registerMesh(name,model);
        return model;
    }
    Ref<Model> MeshManager::createModel(const std::string &name, const std::vector<Vertex> &vertices, const BufferLayout &layout) {
        VC_PROFILER_FUNCTION();
        Ref<Model> model = createRef<Model>(Model::create(vertices,layout));
        instance().add(name,model);
        Application::instance().window().getGraphicContext().registerMesh(name,model);
        return model;
    }

    void MeshManager::add(const std::string& name, Ref<Model> model) {
        p_cache.emplace(name, model);
        VC_CORE_INFO("Model \"{}\" registered",name);
    }

    Ref<Model> MeshManager::get(const std::string& name) {
        const auto it = p_cache.find(name);
        if (it == p_cache.end()) {
            VC_CORE_ERROR("Model with the name \"{}\" doesn't exist", name);
        }
        return it->second;
    }

    bool MeshManager::remove(const std::string& name) {
        return p_cache.erase(name);
    }
} // Vectrix