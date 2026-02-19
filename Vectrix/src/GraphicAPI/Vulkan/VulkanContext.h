#pragma once

#include "vk_mem_alloc.h"
#include "Vectrix/Renderer/GraphicsContext.h"
#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "GraphicAPI/Vulkan/Rendering/VulkanRenderer.h"
#include "Rendering/Shaders/VulkanShaderCompiler.h"

#define VC_VK_CHECK(x,...) if (x!=VK_SUCCESS) {VC_CORE_ERROR(__VA_ARGS__);}

namespace Vectrix {

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		~VulkanContext() override;
		void init() override;
		void swapBuffers() override;


		[[nodiscard]] Device& getDevice() const { return *m_device; }
		[[nodiscard]] VulkanRenderer& getRenderer() const { return *m_renderer; }
		[[nodiscard]] VulkanShaderCompiler& getCompiler() const { return *m_compiler;}
		[[nodiscard]] VmaAllocator getAllocator() const { return getDevice().getAllocator();}
		static VulkanContext& instance() { return *s_instance; }
		static void check_vk_result(VkResult err) {
			if (err == VK_SUCCESS)
				return;

			VC_CORE_ERROR("VkResult = {0}\n", string_VkResult(err));
		}
	private:
		GLFWwindow* m_WindowHandle;
		Own<Device> m_device;
		Own<VulkanRenderer> m_renderer;
		Own<VulkanShaderCompiler> m_compiler;
		
		friend class Shader;
		friend class VulkanVertexBuffer;
		friend class VulkanIndexBuffer;
		friend class Application;
	private:
		static VulkanContext* s_instance;

	};
}
