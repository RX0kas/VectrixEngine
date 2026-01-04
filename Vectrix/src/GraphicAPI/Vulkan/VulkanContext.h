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
		[[nodiscard]] Renderer& getRenderer() const { return *p_renderer; }
		static VulkanContext& instance() { return *s_instance; }
		std::vector<VulkanVertexBuffer*>& getVertexBuffers() {return p_VertexBuffer;}
	private:
		GLFWwindow* p_WindowHandle;
		std::unique_ptr<Device> p_device;
		std::unique_ptr<Renderer> p_renderer;
		
		friend class Shader;
		friend class VulkanVertexBuffer;
		friend class VulkanIndexBuffer;
		friend class Application;
		std::vector<VulkanVertexBuffer*> p_VertexBuffer;
		std::vector<VulkanIndexBuffer*> p_IndexBuffer;
		void render();
	private:
		static VulkanContext* s_instance;

	};

}
