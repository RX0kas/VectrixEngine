#include "vcpch.h"
#include "RenderCommand.h"

#include "GraphicAPI/Vulkan/VulkanRendererAPI.h"

namespace Vectrix {

	RendererAPI* RenderCommand::s_RendererAPI = new VulkanRendererAPI();
	bool RenderCommand::frameStarted = false;
}