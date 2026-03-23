#pragma once
#include "Mesh/Vertex.h"
#include "Vectrix/Core/Log.h"

#include <algorithm>
#include <utility>

namespace Vectrix {

	enum class ShaderDataType {
		None = 0,
		Float, Float2, Float3, Float4,
	};

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

	struct BufferElement {
		std::string name;
		ShaderDataType type;
		uint32_t size;
		uint32_t offset;

		BufferElement(ShaderDataType type, std::string name)
			: name(std::move(name)), type(type), size(ShaderDataTypeSize(type)), offset(0) {}

		bool operator==(const BufferElement& e) const {
			return e.name==this->name && e.type==this->type && e.size==this->size && e.offset==this->offset;
		}
	};

	class BufferLayout {
	public:
		BufferLayout() = default;

		BufferLayout(std::initializer_list<BufferElement> elements)	: m_elements(elements) {
			CalculateOffsetsAndStride();
		}

		[[nodiscard]] uint32_t getStride() const { return m_stride; }
		[[nodiscard]] const std::vector<BufferElement>& getElements() const { return m_elements; }
		[[nodiscard]] bool has(const std::string& name) const {
			const auto end = m_elements.end();
			return std::any_of(m_elements.begin(), end, [name](const BufferElement& x) { return x.name == name; });
		}
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


	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void bind() = 0;

		[[nodiscard]] virtual const BufferLayout& getLayout() const = 0;
		virtual void setLayout(const BufferLayout& layout) = 0;

		static VertexBuffer* create(const std::vector<Vertex>& vertices, uint32_t size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void bind() = 0;

		[[nodiscard]] virtual uint32_t getCount() const = 0;

		static IndexBuffer* create(uint32_t* indices, uint32_t size);
	};


}
