#include "ComputeShaderManager.h"

#include <utility>

#include "Vectrix/Core/Log.h"

namespace Vectrix {


    ComputeShaderManager::ComputeShaderManager() {
        VC_CORE_INFO("Initializing ComputeShaderManager");
    }


    std::shared_ptr<ComputeShader> ComputeShaderManager::createShader(const std::string &name, const std::string &path) {
        VC_CORE_ASSERT(!exist(name),"A compute shader with the name {} already exist",name);
        std::shared_ptr<ComputeShader> shader(ComputeShader::create(name, path));
        if (!shader) return nullptr;
        add(name,shader);
        return shader;
    }

    std::shared_ptr<ComputeShader> ComputeShaderManager::createShaderFromSource(const std::string &name, const std::string &source) {
        VC_CORE_ASSERT(!exist(name),"A compute shader with the name {} already exist",name);
        std::shared_ptr<ComputeShader> shader(ComputeShader::createFromSource(name, source));
        if (!shader) return nullptr;
        add(name,shader);
        return shader;
    }

    ComputeShaderManager::~ComputeShaderManager() {
        VC_CORE_INFO("Destroying ComputeShaderManager");
        m_cache.clear();
    }

    void ComputeShaderManager::add(const std::string& name, std::shared_ptr<ComputeShader> shader) {
        m_cache.emplace(name, std::move(shader));
        VC_CORE_INFO("Compute shader \"{}\" registered",name);
    }

    std::shared_ptr<ComputeShader> ComputeShaderManager::get(const std::string& name) {
        const auto it = m_cache.find(name);
        if (it == m_cache.end()) {
            VC_CORE_ERROR("Compute shader with the name \"{}\" doesn't exist", name);
            return nullptr;
        }
        return it->second;

    }

    bool ComputeShaderManager::remove(const std::string& name) {
        return m_cache.erase(name);
    }
} // Vectrix