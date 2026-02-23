#ifndef VECTRIXWORKSPACE_TEXTURE_MANAGER_H
#define VECTRIXWORKSPACE_TEXTURE_MANAGER_H
#include <unordered_map>
#include "Vectrix/Core/Core.h"
#include "Vectrix/Rendering/Textures/Texture.h"

#include "Vectrix/Utils/Hashing.h"

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
        ~TextureManager();


        /**
         * @brief This function is used to get a texture previously created
         * @pre A texture with this name must exist
         * @param name Name of the texture you wanna get
         * @return The texture program with the name you entered
         */
        Ref<Texture> get(const std::string& name);
        static Ref<Texture> getNotFoundTexture();

        /**
         * @brief This function is used to get all the texture previously created
         * @return The texture programs created
         */
        std::vector<Ref<Texture>> getAll() {
            std::vector<Ref<Texture>> vals;
            vals.reserve(p_cache.size());

            for(const auto&[fst, snd] : p_cache) {
                vals.push_back(snd);
            }
            return vals;
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
        static void createTexture(const std::string &name, const std::string &path);
    private:
        friend class Texture;
        friend class Application;
        TextureManager();
        bool remove(const std::string& name);
        void add(const std::string& name,Ref<Texture> texture);
        std::unordered_map<std::string,Ref<Texture>,XXH3> p_cache;
        Ref<Texture> m_notFoundTexture;

    private:
        static TextureManager* p_instance;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_TEXTURE_MANAGER_H