#ifndef VECTRIXWORKSPACE_TEXTURE_H
#define VECTRIXWORKSPACE_TEXTURE_H
#include <cstdint>

#include "Vectrix/Core/Core.h"

/**
 * @file Texture.h
 * @brief Definition of the texture class
 * @ingroup textures
 */

namespace Vectrix {
    /**
     * @details This class represent a texture that can be created with Vectrix::TextureManager::createTexture() <br>
     * And Vectrix::TextureManager::get() can be used to obtain a texture already created
     */
    class Texture
    {
    public:
        virtual ~Texture() = default;

        /**
         * @brief This function return the width in pixel of the texture
         * @return Width of the texture
         */
        [[nodiscard]] virtual uint32_t getWidth() const = 0;
        /**
         * @brief This function return the height in pixel of the texture
         * @return Height of the texture
         */
        [[nodiscard]] virtual uint32_t getHeight() const = 0;

    private:
        static Ref<Texture> create(const std::string& path);
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_TEXTURE_H