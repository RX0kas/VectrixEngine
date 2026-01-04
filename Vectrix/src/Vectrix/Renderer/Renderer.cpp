#include "vcpch.h"
#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::BeginScene(PerspectiveCamera& camera)
	{
		RenderCommand::setupScene();
		m_SceneData->camera = &camera;
	}

	void Renderer::EndScene()
	{


		RenderCommand::sendScene();
	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray)
	{
		shader->bind();
		// TODO: find a solution to isolate that sort of things
		m_SceneData->camera->p_ubo.sendPush(VulkanContext::instance().getRenderer().getCurrentCommandBuffer(),shader->_pipelineLayout);
		vertexArray->bind();
		RenderCommand::drawIndexed(vertexArray);
	}
}
