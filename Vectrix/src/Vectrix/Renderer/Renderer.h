#pragma once

#include "Vectrix/Renderer/RendererAPI.h"

#include "PerspectiveCamera.h"
#include "Shader.h"
#include "VertexArray.h"

namespace Vectrix {
	class Renderer
	{
	public:
		struct SceneData
		{
			PerspectiveCamera* camera;
		};
		static void BeginScene(PerspectiveCamera& camera);
		static void EndScene();
		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray);

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static SceneData& getSceneData() {return *m_SceneData;}
	private:
		static std::unique_ptr<SceneData> m_SceneData;
	};

}
