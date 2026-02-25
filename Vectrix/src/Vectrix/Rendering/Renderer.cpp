#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	Own<Renderer::SceneData> Renderer::m_SceneData = std::make_unique<Renderer::SceneData>();

	void Renderer::beginScene(PerspectiveCamera& camera)
	{
		m_SceneData->camera = &camera;
	}

	void Renderer::endScene()
	{

	}

	void Renderer::submit(Shader& shader,const VertexArray& vertexArray,const Transform& transform)
	{
		shader.bind();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanRenderer::Submit(shader,vertexArray,transform);
		} else {
			VC_CORE_ERROR("Can't submit a vertex array because the renderer API is set to an unsuported value");
		}
	}

	void Renderer::submit(Shader& shader, Model& model)
	{
		shader.setModelMatrix(model.getModelMatrix());
		if (shader.isAffectedByCamera())
			shader.sendCameraUniform(m_SceneData->camera->getTransformationMatrix());
		submit(shader,*model.getVertexArray(),model.getTransform());
	}
}
