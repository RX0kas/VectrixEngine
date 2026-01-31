#ifndef VECTRIXWORKSPACE_SHADERMANAGER_H
#define VECTRIXWORKSPACE_SHADERMANAGER_H
#include <unordered_map>
#include "Vectrix/Core/Core.h"
#include "Vectrix/Renderer/Buffer.h"
#include "Vectrix/Renderer/Shaders/Shader.h"

#include "Vectrix/Utils/Hashing.h"

namespace Vectrix {

    class ShaderManager {
    public:
        ShaderManager();
        ~ShaderManager();
        Ref<Shader> get(const std::string& name);
        static ShaderManager& instance() { return *p_instance; }
        static void createShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath,ShaderUniformLayout uniformLayout, BufferLayout layout,bool affectedByCamera);
    private:
        friend class Shader;
        bool remove(const std::string& name);
        void add(const std::string& name,Ref<Shader> shader);
        std::unordered_map<std::string,Ref<Shader>,XXH3> p_cache;

    private:
        static ShaderManager* p_instance;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SHADERMANAGER_H