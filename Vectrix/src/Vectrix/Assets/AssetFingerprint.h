#ifndef VECTRIXWORKSPACE_ASSETFINGERPRINT_H
#define VECTRIXWORKSPACE_ASSETFINGERPRINT_H
#include <cstdint>

namespace Vectrix {
    /**
     * @brief What identifies an asset file by its content
     *
     * Scenes store it next to each asset path, so an asset moved or renamed outside the editor can be found
     * again (see AssetsManager::findMovedAsset).
     * @ingroup tools
     */
    struct AssetFingerprint {
        std::uint64_t size = 0; ///< In bytes
        std::uint64_t hash = 0; ///< XXH3 of the content, 0 when unknown

        [[nodiscard]] bool isKnown() const { return hash != 0; }
        bool operator==(const AssetFingerprint&) const = default;
    };
}

#endif //VECTRIXWORKSPACE_ASSETFINGERPRINT_H
