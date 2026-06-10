#ifndef VECTRIXWORKSPACE_SHADERMANAGER_H
#define VECTRIXWORKSPACE_SHADERMANAGER_H
#include "Vectrix/Core/Core.h"
#include "Vectrix/Rendering/Mesh/ObjLoader.h"
#include "Vectrix/Rendering/Shaders/Shader.h"


/**
 * @file ShaderManager.h
 * @brief Definition of the class that manage the life of shaders
 * @ingroup shaders
 */

namespace Vectrix {
    /**
     * @brief Manages shaders
     */
    class ShaderManager {
    public:

        /**
         * @brief This function is used to get a shader previously created
         * @pre A shader with this name must exist
         * @param name Name of the shader you wanna get
         * @return The shader program with the name you entered
         */
        std::shared_ptr<Shader> get(const std::string& name);

        /**
         * @brief This function is used to know if a shader with the name given exist
         * @param name Name of the shader
         * @return If the shader exist in the cache
         */
        bool exist(const std::string& name) {
            return m_cache.find(name) != m_cache.end();
        }

        /**
         * @brief This function is used to get all the shader previously created
         * @return The shader programs created
         */
        std::vector<std::shared_ptr<Shader>> getAll() {
            return m_cache.getAllItems();
        }

        /**
         * @brief Function to create a new shader
         * @pre A shader with the name given should not already exist
         * @param name Name given to the shader
         * @param path Path of the shader source file
         * @param affectedByCamera Tell if we need to send the information of the camera to the GPU
         */
        std::shared_ptr<Shader> createShader(const std::string &name, const std::string &path, const bool affectedByCamera=true);
        ~ShaderManager();
    private:
        friend class Shader;
        friend class AssetsManager;
        ShaderManager();
        bool remove(const std::string& name);
        void add(const std::string& name,std::shared_ptr<Shader> shader);
        Cache<std::string,std::shared_ptr<Shader>> m_cache;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SHADERMANAGER_H