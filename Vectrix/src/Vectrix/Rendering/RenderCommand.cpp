#include "vcpch.h"
#include "RenderCommand.h"

#include "GraphicAPI/Vulkan/VulkanRendererAPI.h"

namespace Vectrix {

	RendererAPI* RenderCommand::s_rendererAPI = new VulkanRendererAPI();
	bool RenderCommand::s_frameStarted = false;
}