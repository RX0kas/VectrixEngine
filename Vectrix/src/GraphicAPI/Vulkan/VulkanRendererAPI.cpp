#include "vcpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"


namespace Vectrix {

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		//glClearColor(color.r, color.g, color.b, color.a);
		VulkanContext::instance().getRenderer().makeClearColor(color);
	}

	void VulkanRendererAPI::Clear()
	{
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		VC_CORE_WARN("VulkanRendererAPI::Clear() is not implemented");
	}

	void VulkanRendererAPI::DrawIndexed(const std::vector<Vertex>& vertexArray)
	{
		//glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

}