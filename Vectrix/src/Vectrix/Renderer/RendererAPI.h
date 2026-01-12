#pragma once

#include <glm/glm.hpp>
#include "Models/VertexArray.h"

namespace Vectrix {

	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API
		{
			None = 0, Vulkan = 1
		};
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual bool setupScene() = 0;
		virtual void sendScene() = 0;

		virtual void DrawIndexed(const VertexArray& vertexArray) = 0;

		static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};

}
