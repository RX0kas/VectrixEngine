#include "Renderer.h"

#include "vcpch.h"
#include "Renderer.h"

#include "RenderCommand.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"

namespace Vectrix {
	std::unique_ptr<Renderer::SceneData> Renderer::m_SceneData = std::make_unique<Renderer::SceneData>();

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

	void Renderer::Submit(const Shader& shader,const VertexArray& vertexArray)
	{
		shader.bind();
		// TODO: find a solution to isolate that sort of things
		CameraPush::sendPush(VulkanContext::instance().getRenderer().getCurrentCommandBuffer(),shader._pipelineLayout,Transform{glm::vec3(0.0f),glm::vec3(1.0f),glm::vec3(0.0f)});
		vertexArray.bind();
		RenderCommand::drawIndexed(vertexArray);
	}

	void Renderer::Submit(const Shader& shader,const VertexArray& vertexArray,const Transform& transform)
	{
		shader.bind();
		// TODO: find a solution to isolate that sort of things
		CameraPush::sendPush(VulkanContext::instance().getRenderer().getCurrentCommandBuffer(),shader._pipelineLayout,transform);
		vertexArray.bind();
		RenderCommand::drawIndexed(vertexArray);
	}

	void Renderer::Submit(const Shader& shader, const Model& model)
	{
		Submit(shader,*model.getVertexArray(),model.getTransform());
	}
}
