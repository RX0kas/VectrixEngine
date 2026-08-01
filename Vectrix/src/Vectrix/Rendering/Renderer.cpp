#include "Renderer.h"

#include "RenderCommand.h"
#include "Camera/EditorCamera.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
	std::unique_ptr<Renderer::SceneData> Renderer::m_SceneData = std::make_unique<SceneData>();
	bool Renderer::s_sceneInProgress = false;

	void Renderer::beginScene(Camera& camera) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!isASceneInProgress(),"Can't begin a scene because another one is in progress");
		s_sceneInProgress = true;
		m_SceneData->transformation_matrix = camera.getTransformationMatrix();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanContext::instance().getRenderer().resetCache();
		} else {
			VC_CORE_ERROR("Can't begin a scene because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::beginScene(EditorCamera& camera) {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!isASceneInProgress(),"Can't begin a scene because another one is in progress");
		s_sceneInProgress = true;
		m_SceneData->transformation_matrix = camera.getTransformationMatrix();
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
			VulkanRenderer::submit(shader,vertexArray,modelMatrix,textureIndex);
		} else {
			VC_CORE_ERROR("Can't submit a vertex array because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::renderOutline(const std::shared_ptr<Entity>& entity, const std::shared_ptr<Framebuffer>& framebuffer) {
		VC_PROFILER_FUNCTION();
		if (!entity) return;
		// <3
		if (!entity->hasComponent<MeshRendererComponent>()) return;

		if (!entity->getComponent<MeshRendererComponent>().isEnable()) return;


		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanContext::instance().getRenderer().renderOutline(entity, framebuffer);
		} else {
			VC_CORE_ERROR("Can't renderOutline because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::resizeMask(glm::vec2 size) {
		VC_PROFILER_FUNCTION();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanContext::instance().getRenderer().resizeMask(size);
		} else {
			VC_CORE_ERROR("Can't resize mask because the renderer API is set to an unsupported value");
		}
	}

	void Renderer::initOutline() {
		VC_PROFILER_FUNCTION();
		if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
			VulkanContext::instance().getRenderer().initOutline();
		} else {
			VC_CORE_ERROR("Can't init outline because the renderer API is set to an unsupported value");
		}
	}
}
