#pragma once

#include "RendererAPI.h"
#include "Models/VertexArray.h"

namespace Vectrix
{
	class RenderCommand
	{
	public:
		static void setClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		static void clear()
		{
			s_RendererAPI->Clear();
		}

		static void drawIndexed(const VertexArray& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		friend class Renderer;

		static bool setupScene() {
			return s_RendererAPI->setupScene();
		}
		static void sendScene() {
			s_RendererAPI->sendScene();
		}
		static RendererAPI* s_RendererAPI;
	};
}

