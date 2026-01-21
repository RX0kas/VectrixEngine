#ifndef VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H
#define VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H

#include "Vectrix/Utils/Memory.h"
#include "ShaderUniform.h"

#include <string>
#include <vector>



namespace Vectrix {
    struct UniformElement {
        std::string name;
        ShaderUniformType type;
        uint32_t offset; // offset dans la struct (std430)
        uint32_t size;   // taille utile (UniformSizeInBytes)
    };

    class ShaderUniformLayout {
    public:
        ShaderUniformLayout() = default;

        // Ajoute un champ dans l'ordre d'apparition (doit correspondre au GLSL)
        void add(const std::string& name, ShaderUniformType type) {
            elements.push_back({name, type, 0, UniformSizeInBytes(type)});
        }

        // calcule offsets selon std430 simplifié
        void finalize() {
            uint32_t cursor = 0;
            for (auto &e : elements) {
                uint32_t a = UniformStd430Alignment(e.type);
                cursor = ALIGN_TO(cursor, a);
                e.offset = cursor;
                // reserve physical stride: vec3 occupies 16 bytes
                uint32_t physicalSize = (e.type == ShaderUniformType::Vec3) ? 16 : ALIGN_TO(e.size, a);
                if (e.type == ShaderUniformType::Mat4) physicalSize = 16 * 4;
                cursor += physicalSize;
            }
            structSize = cursor;
            m_isFinalized = true;
        }

        uint32_t getStructSize() const { return structSize; }

        const UniformElement* find(const std::string& name) const {
            for (auto &e : elements) if (e.name == name) return &e;
            return nullptr;
        }

        bool has(const std::string& name) const {
            for (auto &e : elements) if (e.name == name) return true;
            return false;
        }

        const std::vector<UniformElement>& getElements() const { return elements; }

        bool isFinalized() const {return m_isFinalized;}
    private:
        bool m_isFinalized = false;
        std::vector<UniformElement> elements;
        uint32_t structSize = 0;
    };
}

#endif //VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H