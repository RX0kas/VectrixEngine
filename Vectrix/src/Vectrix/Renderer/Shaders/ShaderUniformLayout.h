#ifndef VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H
#define VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H

#include "Vectrix/Utils/Memory.h"
#include "Vectrix/Core/Log.h"
#include "ShaderUniform.h"

#include <string>
#include <vector>



namespace Vectrix {
    struct UniformElement {
        std::string name;
        ShaderUniformType type;
        uint32_t offset; // std430 offset
        uint32_t size;   // useful size (copied bytes)
    };

    class ShaderUniformLayout {
    public:
        void add(const std::string& name, ShaderUniformType type) {
            VC_CORE_ASSERT(!m_isFinalized, "Cannot add after finalize()");
            elements.push_back({name, type, 0, uniformSizeInBytes(type)});
        }

        void finalize() {
            uint32_t cursor = 0;

            for (auto& e : elements) {
                uint32_t align = uniformAlignment(e.type);
                cursor = ALIGN_TO(cursor, align);

                e.offset = cursor;

                cursor += occupiedSize(e.type);
            }

            structSize = ALIGN_TO(cursor, 16);
            m_isFinalized = true;
        }

        uint32_t getStructSize() const {
            VC_CORE_ASSERT(m_isFinalized, "ShaderUniformLayout not finalized");
            return structSize;
        }

        const UniformElement* find(const std::string& name) const {
            for (auto& e : elements) {
                if (e.name == name) {
                    return &e;
                }
            }
            return nullptr;
        }

        const std::vector<UniformElement>& getElements() const { return elements; }

        bool isFinalized() const { return m_isFinalized; }

    private:
        uint32_t occupiedSize(ShaderUniformType t) {
            switch (t) {
                case ShaderUniformType::Float:
                case ShaderUniformType::Int:
                case ShaderUniformType::Uint:
                case ShaderUniformType::Bool: return 4;
                case ShaderUniformType::Vec2: return 8;
                case ShaderUniformType::Vec3:
                case ShaderUniformType::Vec4: return 16;
                case ShaderUniformType::Mat4: return 64;
                default: return 4;
            }
        }

    private:
        bool m_isFinalized = false;
        std::vector<UniformElement> elements;
        uint32_t structSize = 0;
    };
}

#endif //VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H