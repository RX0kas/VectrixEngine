#include "Renderer.h"

#include "vcpch.h"
#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	Ref<Renderer::SceneData> Renderer::m_SceneData = std::make_unique<Renderer::SceneData>();

	// Return true if a scene can be rendered
	bool Renderer::BeginScene(PerspectiveCamera& camera)
	{
		m_SceneData->camera = &camera;
		return RenderCommand::setupScene();
	}

	void Renderer::EndScene()
	{


		RenderCommand::sendScene();
	}

	void Renderer::Submit(Shader& shader,const VertexArray& vertexArray,const Transform& transform)
	{
		shader.bind();
		if (RendererAPI::GetAPI()==RendererAPI::API::Vulkan) {
			VulkanRenderer::Submit(shader,vertexArray,transform);
		} else {
			VC_CORE_ERROR("Can't submit a vertex array because the renderer API is set to an unsuported value");
		}
	}

	void Renderer::Submit(Shader& shader, const Model& model)
	{
		Submit(shader,*model.getVertexArray(),model.getTransform());
	}
}
