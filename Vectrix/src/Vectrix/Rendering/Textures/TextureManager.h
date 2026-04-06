#ifndef VECTRIXWORKSPACE_TEXTURE_MANAGER_H
#define VECTRIXWORKSPACE_TEXTURE_MANAGER_H
#include "Vectrix/Rendering/Textures/Texture.h"

#include "Vectrix/Utils/Hashing.h"
#include "Vectrix/Utils/Memory.h"

/**
 * @file TextureManager.h
 * @brief Definition of the class that manage the life of textures
 * @ingroup textures
 */

namespace Vectrix {
    /**
     * @brief Manages textures
     */
    class TextureManager {
    public:


        /**
         * @brief This function is used to get a texture previously created
         * @pre A texture with this name must exist
         * @param name Name of the texture you wanna get
         * @return The texture program with the name you entered
         */
        std::shared_ptr<Texture> get(const std::string& name);

        /**
         * @brief Use when a texture can't be found or is not compatible
         * This function return a Ref to a texture that is used by the engine when it cannot found a texture
         * @return The default texture
         */
        static std::shared_ptr<Texture> getNotFoundTexture();

        /**
         * @brief This function is used to get all the texture previously created
         * @return The texture programs created
         */
        std::vector<std::shared_ptr<Texture>> getAll() {
            return m_cache.getAllItems();
        }

        /**
         * @brief This function is used to get all the texture previously created with their names
         * @return The texture programs created
         */
        std::vector<std::pair<std::string,std::shared_ptr<Texture>>> getAllWithName() {
            return m_cache.getAll();
        }

        /**
         * @brief Return a reference to the instance of this class
         * @return The instance of the class TextureManager
         */
        static TextureManager& instance() { return *p_instance; }

        /**
         * @brief Function to create a new texture
         * @param name Name given to the texture
         * @param path Path of the texture file
         */
        static std::shared_ptr<Texture> createTexture(const std::string &name, const std::string &path);
        ~TextureManager();
    private:
        friend class Texture;
        friend class Application;
        TextureManager();
        bool remove(const std::string& name);
        void add(const std::string& name,std::shared_ptr<Texture> texture);
        Cache<std::string,std::shared_ptr<Texture>> m_cache;
        std::shared_ptr<Texture> m_notFoundTexture;

    private:
        static TextureManager* p_instance;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_TEXTURE_MANAGER_H