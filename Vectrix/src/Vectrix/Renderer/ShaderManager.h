#ifndef VECTRIXWORKSPACE_SHADERMANAGER_H
#define VECTRIXWORKSPACE_SHADERMANAGER_H
#include <unordered_map>

#include "GraphicAPI/Vulkan/rendering/Pipeline.h"
#include "Vectrix/Renderer/Shader.h"

#include "Vectrix/Utils/Hashing.h"

namespace Vectrix {

    class ShaderManager {
    public:
        ShaderManager();
        ~ShaderManager();
        std::shared_ptr<Shader> get(const std::string& name);
        static ShaderManager& instance() { return *p_instance; }
        static void createShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, BufferLayout layout);
    private:
        friend class Shader;
        friend class VulkanContext;
        bool remove(const std::string& name);
        void add(const std::string& name,std::shared_ptr<Shader> shader);
        std::unordered_map<std::string,std::shared_ptr<Shader>,XXH3> p_cache;

    private:
        static ShaderManager* p_instance;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SHADERMANAGER_H