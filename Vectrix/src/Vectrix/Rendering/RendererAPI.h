#pragma once

#include <glm/glm.hpp>
#include "Mesh/VertexArray.h"

namespace Vectrix {

	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API
		{
			None = 0, Vulkan = 1
		};
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual bool prepareFrame() = 0;
		virtual void endFrame() = 0;
		virtual void sendFrame() = 0;

		virtual void drawIndexed(const VertexArray& vertexArray) = 0;

		static API getAPI() { return s_API; }
	private:
		static API s_API;
	};

}
