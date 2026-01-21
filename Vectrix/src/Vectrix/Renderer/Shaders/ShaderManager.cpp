#include "Vectrix/Renderer/Shaders/ShaderManager.h"

namespace Vectrix {
    ShaderManager* ShaderManager::p_instance = nullptr;


    ShaderManager::ShaderManager() {
        VC_CORE_INFO("Initializing ShaderManager");
        VC_CORE_ASSERT(!p_instance, "ShaderManager already exists!");
        p_instance = this;
        p_cache.reserve(256);
        p_cache.max_load_factor(0.7f);
    }

    ShaderManager::~ShaderManager() {
        p_cache.clear();
    }



    void ShaderManager::createShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath,ShaderUniformLayout uniformLayout, BufferLayout layout) {
        if (!uniformLayout.isFinalized()) {
            VC_CORE_WARN("Trying to create a shader with a non finalized uniform layout");
            uniformLayout.finalize();
        }

        Ref<Shader> shader(Shader::create(name, vertexPath, fragmentPath,uniformLayout, layout));
        instance().add(name,std::move(shader));
    }

    void ShaderManager::add(const std::string& name, Ref<Shader> shader) {
        p_cache.emplace(name, std::move(shader));
        VC_CORE_INFO("Shader \"{}\" registered",name);
    }

    Ref<Shader> ShaderManager::get(const std::string& name) {
        auto it = p_cache.find(name);
        if (it == p_cache.end()) {
            VC_CORE_ERROR("Shader with the name \"{}\" doesn't exist", name);
        }
        return it->second;

    }

    bool ShaderManager::remove(const std::string& name) {
        return p_cache.erase(name);
    }
} // Vectrix