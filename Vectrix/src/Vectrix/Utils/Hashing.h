#ifndef VECTRIXWORKSPACE_HASHING_H
#define VECTRIXWORKSPACE_HASHING_H

#include <string>
#include <xxh3.h>
#include <functional>

#include "Vectrix/Renderer/Models/Vertex.h"

struct XXH3 {
    static constexpr uint64_t seed = 0x64AC0CB07C37A232;

    std::size_t operator()(const std::string& k) const noexcept {
        return XXH3_64bits_withSeed(k.data(),k.length(),seed);
    }
};
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}


template<>
struct std::hash<Vectrix::Vertex>
{
    size_t operator()(Vectrix::Vertex const& v) const noexcept
    {
        size_t seed = 0;

        hashCombine(seed,
                    v.position.x, v.position.y, v.position.z,
                    v.normal.x,   v.normal.y,   v.normal.z,
                    v.uv.x,       v.uv.y
        );

        return seed;
    }
};

#endif //VECTRIXWORKSPACE_HASHING_H