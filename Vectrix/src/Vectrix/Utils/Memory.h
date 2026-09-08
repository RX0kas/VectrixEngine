#ifndef VECTRIXWORKSPACE_MEMORY_H
#define VECTRIXWORKSPACE_MEMORY_H
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Vectrix/Core/Log.h"
#include "Vectrix/Utils/Hashing.h"

/**
 * @file Memory.h
 * @brief The Cache container and the small memory helpers built on it
 * @ingroup utils
 */

namespace Vectrix {
/**
 * @brief Round an offset up to the next multiple of an alignment
 *
 * Used when packing data for the GPU, where a member has to start on a boundary the API
 * requires.
 * @param offset The offset to round up
 * @param align The alignment to respect, a power of two
 * @ingroup utils
 */
#define ALIGN_TO(offset,align) (offset + (align - 1)) & ~(align - 1)

    /**
     * @brief A hash map tuned for the engine, used to keep loaded assets around
     *
     * It is an `std::unordered_map` hashed with XXH3, pre reserved so the usual asset
     * count does not cause a rehash. The managers use it to hand back an asset that was
     * already loaded rather than loading it twice.
     * @tparam K The key type, usually the name or the path of the asset
     * @tparam I The stored type
     * @see XXH3
     * @ingroup utils
     */
    template<typename K,typename I>
    class Cache : public std::unordered_map<K,I,XXH3> {
    public:
        /**
         * @brief Build the cache, reserving room up front
         * @param reservedN How many entries to make room for
         * @param max_load_factor How full it may get before it rehashes
         */
        Cache(int reservedN = 256,float max_load_factor = 0.7f) {
            this->reserve(reservedN);
            this->max_load_factor(max_load_factor);
        }

        ~Cache() {
            this->clear();
        }

        /**
         * @brief Return every stored value, without its key
         * @return A copy of the values, in no particular order
         * @see getAll
         */
        std::vector<I> getAllItems() {
            std::vector<I> vals;
            vals.reserve(this->size());
            for (auto& [key, val] : *this)
                vals.push_back(val);
            return vals;
        }

        /**
         * @brief Return every entry, key and value together
         * @return A copy of the entries, in no particular order
         * @see getAllItems
         */
        std::vector<std::pair<K,I>> getAll() {
            std::vector<std::pair<K,I>> vals;
            vals.reserve(this->size());
            for (auto& [key, val] : *this)
                vals.emplace_back(key, val);
            return vals;
        }

        /**
         * @brief Tell if something is stored under a key
         * @param key The key to look for
         * @return true when the key is in the cache
         */
        bool exist(K key) {
            return this->find(key)!=this->end();
        }
    };

    /**
     * @brief Read a whole text file into a string
     * @param path The path of the file to read
     * @return The content of the file, empty when it could not be opened
     * @note Reading fails loudly through VC_CORE_ERROR when the file cannot be opened
     * @ingroup utils
     */
    inline std::string readUTF8(const std::string& path) {
        std::ifstream fichier(path, std::ios::binary);
        if (!fichier.is_open()) {
            VC_CORE_ERROR("Can't open: {}", path);
        }
        std::stringstream buffer;
        buffer << fichier.rdbuf();
        return buffer.str();
    }
}
#endif //VECTRIXWORKSPACE_MEMORY_H