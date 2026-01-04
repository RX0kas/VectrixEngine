#include "vcpch.h"
#include "Renderer.h"

#include "GraphicAPI/Vulkan/VulkanRendererAPI.h"

namespace Vectrix {
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::BeginScene(PerspectiveCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.getTransformationMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::vector<Vertex>& vertexArray)
	{
		shader->bind();
		shader->uploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);

		//VulkanRendererAPI::DrawIndexed(vertexArray);
	}
}