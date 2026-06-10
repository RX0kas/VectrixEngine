#include "Vectrix/Rendering/Shaders/ShaderManager.h"

#include <utility>

#include "Vectrix/Rendering/Mesh/ObjLoader.h"

namespace Vectrix {


    ShaderManager::ShaderManager() {
        VC_CORE_INFO("Initializing ShaderManager");
    }



    std::shared_ptr<Shader> ShaderManager::createShader(const std::string &name, const std::string &path, const bool affectedByCamera) {
        VC_CORE_ASSERT(!exist(name),"A shader with the name {} already exist",name);
        std::shared_ptr<Shader> shader(Shader::create(name, path, getTinyObjLayout()));
        add(name,shader);
        return shader;
    }

    ShaderManager::~ShaderManager() {
        VC_CORE_INFO("Destroying ShaderManager");
        m_cache.clear();
    }

    void ShaderManager::add(const std::string& name, std::shared_ptr<Shader> shader) {
        m_cache.emplace(name, std::move(shader));
        VC_CORE_INFO("Shader \"{}\" registered",name);
    }

    std::shared_ptr<Shader> ShaderManager::get(const std::string& name) {
        const auto it = m_cache.find(name);
        if (it == m_cache.end()) {
            VC_CORE_ERROR("Shader with the name \"{}\" doesn't exist", name);
        }
        return it->second;

    }

    bool ShaderManager::remove(const std::string& name) {
        return m_cache.erase(name);
    }
} // Vectrix