#pragma once

#include "Vectrix/Renderer/GraphicsContext.h"
#include "GraphicAPI/Vulkan/Rendering/Device.h"
#include "GraphicAPI/Vulkan/Rendering/VulkanRenderer.h"
#include "Rendering/Shaders/VulkanShaderCompiler.h"

namespace Vectrix {

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		void init() override;
		void swapBuffers() override;


		[[nodiscard]] Device& getDevice() const { return *p_device; }
		[[nodiscard]] VulkanRenderer& getRenderer() const { return *p_renderer; }
		[[nodiscard]] VulkanShaderCompiler& getCompiler() const { return *p_compiler;}
		static VulkanContext& instance() { return *s_instance; }
	private:
		GLFWwindow* p_WindowHandle;
		Ref<Device> p_device;
		Ref<VulkanRenderer> p_renderer;
		Own<VulkanShaderCompiler> p_compiler;
		
		friend class Shader;
		friend class VulkanVertexBuffer;
		friend class VulkanIndexBuffer;
		friend class Application;
	private:
		static VulkanContext* s_instance;

	};

}
