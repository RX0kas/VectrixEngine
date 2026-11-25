#ifndef VECTRIXWORKSPACE_COMPUTESHADERMANAGER_H
#define VECTRIXWORKSPACE_COMPUTESHADERMANAGER_H
#include <memory>
#include <string>
#include <vector>

#include "Vectrix/Core/Core.h"
#include "Vectrix/Utils/Memory.h"
#include "Vectrix/Physics/ComputeShader.h"


namespace Vectrix {
    class ComputeShaderManager {
    public:
        std::shared_ptr<ComputeShader> get(const std::string& name);

        bool exist(const std::string& name) {
            return m_cache.find(name) != m_cache.end();
        }

        std::vector<std::shared_ptr<ComputeShader>> getAll() {
            return m_cache.getAllItems();
        }


        std::shared_ptr<ComputeShader> createShader(const std::string &name, const std::string &path);

        std::shared_ptr<ComputeShader> createShaderFromSource(const std::string &name, const std::string &source);
        ~ComputeShaderManager();
    private:
        friend class ComputeShader;
        friend class AssetsManager;
        ComputeShaderManager();
        bool remove(const std::string& name);
        void add(const std::string& name,std::shared_ptr<ComputeShader> shader);
        void clear() { m_cache.clear(); }
        Cache<std::string,std::shared_ptr<ComputeShader>> m_cache;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_COMPUTESHADERMANAGER_H