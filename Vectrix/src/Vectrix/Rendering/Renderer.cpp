#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	Own<Renderer::SceneData> Renderer::m_SceneData = createOwn<SceneData>();

	void Renderer::beginScene(PerspectiveCamera& camera) {
		VC_PROFILER_FUNCTION();
		m_SceneData->camera = &camera;
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanRenderer::resetCache();
		} else {
			VC_CORE_ERROR("Can't begin a scene because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::endScene() {
		VC_PROFILER_FUNCTION();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanContext::instance().getRenderer().flush();
		} else {
			VC_CORE_ERROR("Can't end a scene because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::submit(Shader& shader,Ref<VertexArray> vertexArray,Transform transform) {
		VC_PROFILER_FUNCTION();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanRenderer::submit(shader,vertexArray,transform);
		} else {
			VC_CORE_ERROR("Can't submit a vertex array because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::submit(Shader& shader, Model& model) {
		VC_PROFILER_FUNCTION();
		submit(shader,model.getVertexArray(),model.getTransform());
	}
}
