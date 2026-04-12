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
	enum class ShaderDataType {
		None = 0,
		Float, Float2, Float3, Float4,
	};

	/// @cond INTERNAL
	static uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:  return 4;
			case ShaderDataType::Float2: return 8;
			case ShaderDataType::Float3: return 12;
			case ShaderDataType::Float4:
			default: return 16;
		}
		VC_CORE_ERROR("Unknown ShaderDataType");
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

		BufferElement(const ShaderDataType type, std::string name) : name(std::move(name)), type(type), size(ShaderDataTypeSize(type)), offset(0) {}

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

		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
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


	/**
 	* @brief Abstract interface for a GPU vertex buffer
 	*/
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		/**
		 * @brief Binds the vertex buffer for use in the current draw call
		 **/
		virtual void bind() = 0;

		/**
		 * @brief Returns the buffer layout describing the vertex attributes
		 **/
		[[nodiscard]] virtual const BufferLayout& getLayout() const = 0;

		/**
	     * @brief Set the buffer layout describing the vertex attributes
	     * @param layout The layout to assign to this buffer
	     */
		virtual void setLayout(const BufferLayout& layout) = 0;

		/**
	 	* @brief Function to create a new VertexBuffer
	 	* @param vertices List of vertices to upload to the GPU
	 	* @param size Total size in bytes of the vertex data
	 	* @return A shared pointer to the created vertex buffer
	 	*/
		static std::shared_ptr<VertexBuffer> create(const std::vector<Vertex>& vertices, uint32_t size);
	};


	/**
 	* @brief Abstract interface for a GPU index buffer
 	*/
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		/**
		 * @brief Binds the index buffer for use in the current draw call
		 **/
		virtual void bind() = 0;

		/**
		 * @brief Returns the number of indices stored in this buffer
		 **/
		[[nodiscard]] virtual uint32_t getCount() const = 0;

		/**
		 * @brief Function to create a new IndexBuffer
		 * @param indices Pointer to the array of indices
	 	 * @param size Number of indices in the array
	 	 * @return A shared pointer to the created index buffer
		 */
		static std::shared_ptr<IndexBuffer> create(uint32_t* indices, uint32_t size);
	};


}
