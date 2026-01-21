#pragma once

#include "Vectrix/Renderer/GraphicsContext.h"
#include "GraphicAPI/Vulkan/rendering/Device.h"
#include "GraphicAPI/Vulkan/rendering/VulkanRenderer.h"
#include "GraphicAPI/Vulkan/rendering/VulkanBuffer.h"

namespace Vectrix {

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		void init() override;
		void swapBuffers() override;


		[[nodiscard]] Device& getDevice() const { return *p_device; }
		[[nodiscard]] VulkanRenderer& getRenderer() const { return *p_renderer; }
		static VulkanContext& instance() { return *s_instance; }
	private:
		GLFWwindow* p_WindowHandle;
		Ref<Device> p_device;
		Ref<VulkanRenderer> p_renderer;
		
		friend class Shader;
		friend class VulkanVertexBuffer;
		friend class VulkanIndexBuffer;
		friend class Application;
	private:
		static VulkanContext* s_instance;

	};

}
