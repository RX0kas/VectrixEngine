#pragma once

#include "Vectrix/Renderer/RendererAPI.h"

#include "Camera/PerspectiveCamera.h"
#include "Models/Model.h"
#include "Shaders/Shader.h"
#include "Models/VertexArray.h"

namespace Vectrix {
	class Renderer
	{
	public:
		struct SceneData
		{
			PerspectiveCamera* camera;
		};
		static bool BeginScene(PerspectiveCamera& camera);
		static void EndScene();
		static void Submit(Shader& shader, const VertexArray& vertexArray,const Transform& transform=Transform{glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)});
		static void Submit(Shader& shader, const Model& model);

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static SceneData& getSceneData() {return *m_SceneData;}
	private:
		static Ref<SceneData> m_SceneData;
	};

}
