#pragma once

#include "Vectrix/Renderer/RendererAPI.h"

#include "PerspectiveCamera.h"
#include "Shader.h"

namespace Vectrix {
	class Renderer
	{
	public:
		static void BeginScene(PerspectiveCamera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<Shader>& shader, const std::vector<Vertex>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};

}
