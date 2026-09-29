#ifndef VECTRIXWORKSPACE_HASHING_H
#define VECTRIXWORKSPACE_HASHING_H

#include <string>
#include <xxh3.h>

#include "Vectrix/Rendering/Mesh/Vertex.h"

/**
 * @file Hashing.h
 * @brief The hash functions the engine uses for its caches
 * @ingroup utils
 */

/**
 * @brief Hashes a string with xxHash, for use as a hash map hasher
 *
 * It is what Cache uses instead of `std::hash`, since the keys are mostly asset names
 * and xxHash is quicker on those.
 * @see Vectrix::Cache
 * @ingroup utils
 */
struct XXH3 {
    /// The seed every hash is computed with, so the results stay stable across runs
    static constexpr uint64_t seed = 0x64AC0CB07C37A232;

    /**
     * @brief Hash a string
     * @param k The string to hash
     * @return The hash of the string
     */
    std::size_t operator()(const std::string& k) const noexcept {
        return XXH3_64bits_withSeed(k.data(),k.length(),seed);
    }
};

/**
 * @brief Fold one or more values into an existing hash
 *
 * Used to hash a compound value such as a Vertex, by combining the hash of each of its
 * fields into a single one.
 * @tparam T The type of the first value to fold in
 * @tparam Rest The types of the remaining values
 * @param seed The hash to update, in and out
 * @param v The first value to fold in
 * @param rest The remaining values
 * @ingroup utils
 */
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}


/// @cond INTERNAL
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
/// @endcond

#endif //VECTRIXWORKSPACE_HASHING_H