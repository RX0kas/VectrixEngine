#ifndef VECTRIXWORKSPACE_MEMORY_H
#define VECTRIXWORKSPACE_MEMORY_H
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Vectrix/Core/Log.h"
#include "Vectrix/Utils/Hashing.h"

namespace Vectrix {
#define ALIGN_TO(offset,align) (offset + (align - 1)) & ~(align - 1)

    template<typename K,typename I>
    class Cache : public std::unordered_map<K,I,XXH3> {
    public:
        Cache(int reservedN = 256,float max_load_factor = 0.7f) {
            this->reserve(reservedN);
            this->max_load_factor(max_load_factor);
        }

        ~Cache() {
            this->clear();
        }

        std::vector<I> getAllItems() {
            std::vector<I> vals;
            vals.reserve(this->size());

            for (auto it = this->begin(); it != this->end(); ++it) {
                vals.push_back(it->second);
            }

            return vals;
        }

        std::vector<std::pair<K,I>> getAll() {
            std::vector<std::pair<K,I>> vals;
            vals.reserve(this->size());

            for (auto it = this->begin(); it != this->end(); ++it) {
                vals.emplace_back(it->first, it->second);
            }

            return vals;
        }

        bool exist(K key) {
            return this->find(key)!=this->end();
        }
    };

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