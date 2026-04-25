#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	std::unique_ptr<Renderer::SceneData> Renderer::m_SceneData = std::make_unique<SceneData>();

	void Renderer::beginScene(PerspectiveCamera& camera) {
		VC_PROFILER_FUNCTION();
		m_SceneData->camera = &camera;
	}

	void Renderer::endScene() {
		VC_PROFILER_FUNCTION();
	}

	void Renderer::submit(Shader& shader,const std::shared_ptr<VertexArray>& vertexArray, glm::mat4 modelMatrix,uint32_t textureIndex) {
		VC_PROFILER_FUNCTION();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanRenderer::submit(shader,vertexArray,modelMatrix);
		} else {
			VC_CORE_ERROR("Can't submit a vertex array because the renderer API is set to an unsupported value");
		}
	}
}
