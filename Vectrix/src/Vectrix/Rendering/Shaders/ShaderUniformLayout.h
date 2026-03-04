#ifndef VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H
#define VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H

#include "Vectrix/Utils/Memory.h"
#include "Vectrix/Core/Log.h"
#include "ShaderUniformType.h"

#include <string>
#include <vector>


/**
 * @file ShaderUniformLayout.h
 * @brief Definition of the layout of the information sent to the GPU buffer
 * @ingroup shaders
 */

namespace Vectrix {
    /**
     * Describe an Element of the ShaderUniformLayout
     */
    struct UniformElement {
        /**
         * @brief The name of the element
         */
        std::string name;
        /**
         * @brief The Type of the element
         */
        ShaderUniformType type;
        /**
         * @brief the std430 offset
         */
        uint32_t offset;
    };

    /**
     * A class that contains the name and type of the different values sent to the Shader
     */
    class ShaderUniformLayout {
    public:
        /**
         * @brief This function add a new value that is going to be sent to the shader
         * @param name Name of the value
         * @param type Type of the value
         * @pre The ShaderUniformLayout must not already be sent to a Shader
         */
        void add(const std::string& name, ShaderUniformType type) {
            VC_CORE_ASSERT(!m_isFinalized, "This ShaderUniformLayout has already been sent to a Shader");
            m_elements.push_back({name, type, 0});
        }

        /**
         * @brief Return the size of the information sent by the shader to the GPU
         * @pre The ShaderUniformLayout must have already been sent to a Shader
         * @return The size of the data sent to the GPU
         */
        [[nodiscard]] uint32_t getStructSize() const {
            VC_CORE_ASSERT(m_isFinalized, "This ShaderUniformLayout has not been sent to a Shader");
            return m_structSize;
        }

        /**
         * @brief A function used to get a certain uniform element in the layout
         * @param name Name of the element you wanna get
         * @pre The element must have already been added into the layout
         * @return A pointer to the element with the name you entered or nullptr if it doesn't exist
         */
        [[nodiscard]] const UniformElement* find(const std::string& name) const {
            for (auto& e : m_elements) {
                if (e.name == name) {
                    return &e;
                }
            }
            VC_CORE_WARN("Searching for an uniform element that doesn't exist: {}",name);
            return nullptr;
        }

        /**
         * @brief This is a function to get all the elements you previously added into the layout
         * @return All the UniformElement of the layout
         */
        [[nodiscard]] const std::vector<UniformElement>& getElements() const { return m_elements; }


        /**
         * @brief Return if the ShaderUniformLayout has been sent to a shader, and you can no longer add anything
         * @return True if the ShaderUniformLayout has already been sent to a shader
         */
        [[nodiscard]] bool isFinalized() const { return m_isFinalized; }

    private:
        static uint32_t occupiedSize(const ShaderUniformType t) {
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
        void finalize() {
            uint32_t cursor = 0;

            for (auto& e : m_elements) {
                uint32_t align = uniformAlignment(e.type);
                cursor = ALIGN_TO(cursor, align);

                e.offset = cursor;

                cursor += occupiedSize(e.type);
            }

            m_structSize = ALIGN_TO(cursor, 16);
            m_isFinalized = true;
        }

        friend class Shader;
        bool m_isFinalized = false;
        std::vector<UniformElement> m_elements;
        uint32_t m_structSize = 0;
    };
}

#endif //VECTRIXWORKSPACE_SHADERUNIFORMLAYOUT_H