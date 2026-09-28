#pragma once
#include "Mesh/Vertex.h"
#include "Vectrix/Core/Log.h"

#include <algorithm>
#include <utility>

/**
* @file Buffer.h
* @brief Definition of all the buffers class and internal struct related
* @ingroup mesh
*/

namespace Vectrix {
	/**
	 * @brief The type of one piece of data inside a vertex
	 * @see BufferElement
	 * @ingroup buffers
	 */
	enum class ShaderDataType {
		None = 0, ///< No type
		Float,    ///< A single float
		Float2,   ///< Two floats, a 2D vector
		Float3,   ///< Three floats, a 3D vector
		Float4,   ///< Four floats, a 4D vector or a colour
	};

	/// @cond INTERNAL
	inline uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:  return 4;
			case ShaderDataType::Float2: return 8;
			case ShaderDataType::Float3: return 12;
			case ShaderDataType::Float4: return 16;
			case ShaderDataType::None:   return 0;
		}
		return 0;
	}
	/// @endcond

	/**
	 * @brief Represent a data is represented in a vertex
	 **/
	struct BufferElement {
		/**
		 * @brief The name of the element
		 */
		std::string name;

		/**
		 * @brief The type of the element
		 */
		ShaderDataType type;

		/**
		 * @brief The size of the element in the GPU
		 */
		uint32_t size;

		/**
		 * @brief The offset to find the element
		 */
		uint32_t offset;

		/**
		 * @brief Describe one piece of data of a vertex
		 * @param type The type of the element
		 * @param name The name the shader knows it by
		 * @note The offset is worked out by the BufferLayout holding the element
		 */
		BufferElement(const ShaderDataType type, std::string name) : name(std::move(name)), type(type), size(ShaderDataTypeSize(type)), offset(0) {}

		/**
		 * @brief Tell if two elements describe the same thing
		 * @param e The element to compare with
		 * @return true when the name, the type, the size and the offset all match
		 */
		bool operator==(const BufferElement& e) const {
			return e.name==this->name && e.type==this->type && e.size==this->size && e.offset==this->offset;
		}
	};

	/**
	 * @brief Represent how the vertex data are sent to the shader
	 */
	class BufferLayout {
	public:
		BufferLayout() = default;

		/**
		 * @brief Build the layout from the elements a vertex is made of
		 *
		 * The offset of each element and the stride between two vertices are worked out
		 * from the order the elements are given in.
		 * @param elements The elements, in the order they sit in a vertex
		 */
		BufferLayout(std::initializer_list<BufferElement> elements)	: m_elements(elements) {
			CalculateOffsetsAndStride();
		}

		/**
		 * @brief The distance between each vertex
		 */
		[[nodiscard]] uint32_t getStride() const { return m_stride; }

		/**
		 * @brief Return all the elements in the layout
		 */
		[[nodiscard]] const std::vector<BufferElement>& getElements() const { return m_elements; }

		/**
		 * @brief Tell if an element is in the layout
		 */
		[[nodiscard]] bool has(const std::string& name) const {
			const auto end = m_elements.end();
			return std::any_of(m_elements.begin(), end, [name](const BufferElement& x) { return x.name == name; });
		}

		/**
		 * @brief Return an iterator on the first element, so the layout can be walked
		 * @return An iterator to the beginning of the elements
		 */
		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }

		/**
		 * @brief Return an iterator past the last element
		 * @return An iterator to the end of the elements
		 */
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }
	private:
		void CalculateOffsetsAndStride() {
			uint32_t offset = 0;
			m_stride = 0;

			for (auto& element : m_elements) {
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

	private:
		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;
	};


}
