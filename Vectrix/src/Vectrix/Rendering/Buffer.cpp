#include "vcpch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "GraphicAPI/Vulkan/Rendering/VulkanBuffer.h"
#include "Vectrix/Rendering/RendererAPI.h"



namespace Vectrix {
	VertexBuffer* VertexBuffer::create(const std::vector<Vertex>& vertices, uint32_t size)
	{
		switch (Renderer::getAPI())
		{
			case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI is set to None");
			case RendererAPI::API::Vulkan:  return new VulkanVertexBuffer(vertices, size);
		}

		VC_CORE_ERROR("Unknown RendererAPI!");
	}

	IndexBuffer* IndexBuffer::create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::getAPI())
		{
			case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!");
			case RendererAPI::API::Vulkan:  return new VulkanIndexBuffer(indices, size);
		}

		VC_CORE_ERROR("Unknown RendererAPI!");
	}
}