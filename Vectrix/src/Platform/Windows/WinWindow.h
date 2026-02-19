#pragma once

#include "vcpch.h"
#include "../../Vectrix/Core/Window.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Renderer/GraphicsContext.h"
#include "Vectrix/Renderer/RendererAPI.h"


namespace Vectrix {

	class WinWindow : public Window
	{
		// That's an internal class to handle the window things on Windows
	public:
		WinWindow();
		~WinWindow() override;

		[[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(m_window); }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

		[[nodiscard]] unsigned int getWidth() const override { return m_data.Width; }
		[[nodiscard]] unsigned int getHeight() const override { return m_data.Height; }
		[[nodiscard]] float getAspect() const override {
			if (RendererAPI::getAPI()==RendererAPI::API::Vulkan) {
				return VulkanContext::instance().getRenderer().getAspectRatio();
			}

			VC_CORE_WARN("Using default aspect since RendererAPI is not supported");
			return m_data.Width/m_data.Height;
		}

		void onUpdate() override;

		[[nodiscard]] bool wasWindowResized() const override { return m_data.windowResized; }

		void resetWindowResizedFlag() override { m_data.windowResized = false; }

		[[nodiscard]] void* getNativeWindow() const override { return m_window; }

		void setEventCallback(const EventCallbackFn& callback) override { m_data.EventCallback = callback; }

		void setVSync(bool enabled) override;

		[[nodiscard]] bool isVSync() const override;
		void init(const WindowAttributes& data) override;

		void show() override {
			glfwShowWindow(m_window);
			m_data.visible = true;
		}

		void hide() override {
			glfwHideWindow(m_window);
			m_data.visible = false;
		}

		[[nodiscard]] bool isVisible() const override {
			return m_data.visible;
		}
	private:
		void shutdown();

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		GLFWwindow* m_window;
		Own<GraphicsContext> m_context;

		WindowData m_data;
	};
}
