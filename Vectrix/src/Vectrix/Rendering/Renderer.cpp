#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	std::unique_ptr<Renderer::SceneData> Renderer::m_SceneData = std::make_unique<SceneData>();
	bool Renderer::s_sceneInProgress = false;

	void Renderer::beginScene(Camera& camera) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!isASceneInProgress(),"Can't begin a scene because another one is in progress");
		s_sceneInProgress = true;
		m_SceneData->camera = &camera;
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanContext::instance().getRenderer().resetCache();
		} else {
			VC_CORE_ERROR("Can't begin a scene because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::endScene() {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(isASceneInProgress(),"Can't end a scene because none is in progress");
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanContext::instance().getRenderer().flush();
		} else {
			VC_CORE_ERROR("Can't end a scene because the renderer API is set to an unsupported value");
		}
		s_sceneInProgress = false;
	}

	void Renderer::submit(const std::shared_ptr<Shader>& shader,const std::shared_ptr<VertexArray>& vertexArray, glm::mat4 modelMatrix,uint32_t textureIndex) {
		VC_PROFILER_FUNCTION();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanRenderer::submit(shader,vertexArray,modelMatrix);
		} else {
			VC_CORE_ERROR("Can't submit a vertex array because the renderer API is set to an unsupported value");
		}
	}
}
