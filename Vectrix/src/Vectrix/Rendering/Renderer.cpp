#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	Own<Renderer::SceneData> Renderer::m_SceneData = std::make_unique<SceneData>();

	void Renderer::beginScene(PerspectiveCamera& camera) {
		VC_PROFILER_FUNCTION();
		m_SceneData->camera = &camera;
	}

	void Renderer::endScene() {
		VC_PROFILER_FUNCTION();
	}

	void Renderer::submit(Shader& shader,const VertexArray& vertexArray,Transform transform) {
		VC_PROFILER_FUNCTION();
		shader.setModelMatrix(transform.mat4());
		if (shader.isAffectedByCamera())
			shader.sendCameraUniform(m_SceneData->camera->getTransformationMatrix());
		shader.bind();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanRenderer::Submit(shader,vertexArray,transform);
		} else {
			VC_CORE_ERROR("Can't submit a vertex array because the renderer API is set to an unsuported value");
		}
	}

	void Renderer::submit(Shader& shader, Model& model) {
		VC_PROFILER_FUNCTION();
		submit(shader,*model.getVertexArray(),model.getTransform());
	}
}
