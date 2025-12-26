#pragma once

#include "Vectrix/Renderer/GraphicsContext.h"
#include "GraphicAPI/Vulkan/rendering/Device.h"
#include "GraphicAPI/Vulkan/rendering/VulkanRenderer.h"
#include "GraphicAPI/Vulkan/rendering/VulkanBuffer.h"
#include "GraphicAPI/Vulkan/rendering/Vertex.h"
#include "Vectrix/Renderer/Shader.h"

namespace Vectrix {

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void init() override;
		virtual void swapBuffers() override;


		Device& getDevice() const { return *_device; }
		Renderer& getRenderer() const { return *_renderer; }
		inline static VulkanContext& instance() { return *_instance; }
		std::vector<VulkanVertexBuffer*>& getVertexBuffers() {return _VertexBuffer;}
	private:
		GLFWwindow* _WindowHandle;
		std::unique_ptr<Device> _device;
		std::unique_ptr<Renderer> _renderer;
		
		friend class Shader;
		friend class VulkanVertexBuffer;
		friend class VulkanIndexBuffer;
		friend class Application;
		std::vector<Shader*> _Shaders;
		std::vector<VulkanVertexBuffer*> _VertexBuffer;
		std::vector<VulkanIndexBuffer*> _IndexBuffer;
		void render();
	private:
		static VulkanContext* _instance;

	};

}
