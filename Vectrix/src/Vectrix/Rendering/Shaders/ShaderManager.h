#ifndef VECTRIXWORKSPACE_SHADERMANAGER_H
#define VECTRIXWORKSPACE_SHADERMANAGER_H
#include <unordered_map>
#include "Vectrix/Core/Core.h"
#include "Vectrix/Rendering/Buffer.h"
#include "Vectrix/Rendering/Shaders/Shader.h"

#include "Vectrix/Utils/Hashing.h"

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
        ~ShaderManager();


        /**
         * @brief This function is used to get a shader previously created
         * @pre A shader with this name must exist
         * @param name Name of the shader you wanna get
         * @return The shader program with the name you entered
         */
        Ref<Shader> get(const std::string& name);

        /**
         * @brief This function is used to know if a shader with the name given exist
         * @param name Name of the shader
         * @return If the shader exist in the cache
         */
        static bool exist(const std::string& name) {
            return instance().m_cache.find(name) != instance().m_cache.end();
        }

        /**
         * @brief This function is used to get all the shader previously created
         * @return The shader programs created
         */
        std::vector<Ref<Shader>> getAll() {
            std::vector<Ref<Shader>> vals;
            vals.reserve(m_cache.size());

            for(const auto&[fst, snd] : m_cache) {
                vals.push_back(snd);
            }
            return vals;
        }

        /**
         * @brief Return a reference to the instance of this class
         * @return The instance of the class ShaderManager
         */
        static ShaderManager& instance() { return *s_instance; }

        /**
         * @brief Function to create a new shader
         * @pre A shader with the name given should not already exist
         * @param name Name given to the shader
         * @param vertexPath Path of the vertex source file
         * @param fragmentPath Path of the fragment source file
         * @param uniformLayout Description of the element sent to the GPU
         * @param layout Information on how the vertex are sent to the GPU
         * @param affectedByCamera Tell if we need to send the information of the camera to the GPU
         */
        static void createShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath,ShaderUniformLayout uniformLayout, BufferLayout layout,bool affectedByCamera);
    private:
        friend class Shader;
        friend class Application;
        ShaderManager();
        bool remove(const std::string& name);
        void add(const std::string& name,Ref<Shader> shader);
        std::unordered_map<std::string,Ref<Shader>,XXH3> m_cache;

    private:
        static ShaderManager* s_instance;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SHADERMANAGER_H