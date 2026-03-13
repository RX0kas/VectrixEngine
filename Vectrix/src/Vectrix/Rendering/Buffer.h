#pragma once
#include "Models/Vertex.h"
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
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;

		BufferElement(ShaderDataType type, std::string name)
			: Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)), Offset(0) {}

		bool operator==(const BufferElement& e) const {
			return e.Name==this->Name && e.Type==this->Type && e.Size==this->Size && e.Offset==this->Offset;
		}
	};

	class BufferLayout {
	public:
		BufferLayout() = default;

		BufferLayout(std::initializer_list<BufferElement> elements)	: m_Elements(elements) {
			CalculateOffsetsAndStride();
		}

		[[nodiscard]] uint32_t getStride() const { return m_Stride; }
		[[nodiscard]] const std::vector<BufferElement>& getElements() const { return m_Elements; }
		[[nodiscard]] bool has(const std::string& name) const {
			const auto end = m_Elements.end();
			return std::any_of(m_Elements.begin(), end, [name](const BufferElement& x) { return x.Name == name; });
		}
	private:
		void CalculateOffsetsAndStride() {
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements) {
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
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
