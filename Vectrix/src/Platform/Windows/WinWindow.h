#pragma once

#include "vcpch.h"
#include "Vectrix/Window.h"
#include "Vectrix/Renderer/GraphicsContext.h"

// That's an internal class to handle the window things on Windows
namespace Vectrix {
	class WinWindow : public Window
	{
	public:
		WinWindow();

		bool shouldClose() { return glfwWindowShouldClose(window); }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		VkExtent2D getExtent() { return { static_cast<uint32_t>(data.Width), static_cast<uint32_t>(data.Height) }; }
		inline unsigned int getWidth() const override { return data.Width; }
		inline unsigned int getHeight() const override { return data.Height; }

		void onUpdate() override;

		bool wasWindowResized() { return framebufferResized; }

		void resetWindowResizedFlag() { framebufferResized = false; }

		inline virtual void* getNativeWindow() const { return window; }

		inline void setEventCallback(const EventCallbackFn& callback) override { data.EventCallback = callback; }

		void setVSync(bool enabled) override;

		bool isVSync() const override;
		virtual void init(const WindowAttributes& data = WindowAttributes());
	private:
		virtual void shutdown();

	private:

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		bool framebufferResized;

		GLFWwindow* window;
		GraphicsContext* _context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData data;
	};
}
