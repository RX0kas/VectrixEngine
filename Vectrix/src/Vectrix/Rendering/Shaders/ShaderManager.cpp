#include "Vectrix/Rendering/Shaders/ShaderManager.h"

#include <utility>

namespace Vectrix {
    ShaderManager* ShaderManager::s_instance = nullptr;


    ShaderManager::ShaderManager() {
        VC_CORE_INFO("Initializing ShaderManager");
        VC_CORE_ASSERT(!s_instance, "ShaderManager already exists!");
        s_instance = this;
        m_cache.reserve(256);
        m_cache.max_load_factor(0.7f);
    }

    ShaderManager::~ShaderManager() {
        m_cache.clear();
    }



    void ShaderManager::createShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath,ShaderUniformLayout uniformLayout, BufferLayout layout,bool affectedByCamera) {
        VC_CORE_ASSERT(!exist(name),"A shader with the name {} already exist",name);
        Ref<Shader> shader(Shader::create(name, vertexPath, fragmentPath,std::move(uniformLayout), layout,affectedByCamera));
        instance().add(name,std::move(shader));
    }

    void ShaderManager::add(const std::string& name, Ref<Shader> shader) {
        m_cache.emplace(name, std::move(shader));
        VC_CORE_INFO("Shader \"{}\" registered",name);
    }

    Ref<Shader> ShaderManager::get(const std::string& name) {
        auto it = m_cache.find(name);
        if (it == m_cache.end()) {
            VC_CORE_ERROR("Shader with the name \"{}\" doesn't exist", name);
        }
        return it->second;

    }

    bool ShaderManager::remove(const std::string& name) {
        return m_cache.erase(name);
    }
} // Vectrix