#ifndef VECTRIXWORKSPACE_HASHING_H
#define VECTRIXWORKSPACE_HASHING_H

#include <string>
#include <xxh3.h>

struct XXH3 {
    static constexpr uint64_t seed = 0x64AC0CB07C37A232;

    std::size_t operator()(const std::string& k) const noexcept {
        return XXH3_64bits_withSeed(k.data(),k.length(),seed);
    }
};

#endif //VECTRIXWORKSPACE_HASHING_H