#ifndef VECTRIXWORKSPACE_TEXTURE_H
#define VECTRIXWORKSPACE_TEXTURE_H
#include <cstdint>

#include "Vectrix/Core/Core.h"

namespace Vectrix {
    class Texture
    {
    public:
        virtual ~Texture() = default;

        [[nodiscard]] virtual uint32_t getWidth() const = 0;
        [[nodiscard]] virtual uint32_t getHeight() const = 0;

        virtual void bind(uint32_t slot = 0) const = 0;
        static Ref<Texture> create(const std::string& path);
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_TEXTURE_H