#pragma once

#include "vcpch.h"
#include "Vectrix/Window.h"
#include "Vectrix/Renderer/GraphicsContext.h"


namespace Vectrix {
	class LinWindow : public Window
	{
		// That's an internal class to handle the window things on Linux
	public:
		LinWindow();

		bool shouldClose() { return glfwWindowShouldClose(window); }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		[[nodiscard]] VkExtent2D getExtent() const { return { static_cast<uint32_t>(data.Width), static_cast<uint32_t>(data.Height) }; }
		[[nodiscard]] unsigned int getWidth() const override { return data.Width; }
		[[nodiscard]] unsigned int getHeight() const override { return data.Height; }

		void onUpdate() override;

		[[nodiscard]] bool wasWindowResized() const override { return data.windowResized; }

		void resetWindowResizedFlag() override { data.windowResized = false; }

		[[nodiscard]] void* getNativeWindow() const override { return window; }

		void setEventCallback(const EventCallbackFn& callback) override { data.EventCallback = callback; }

		void setVSync(bool enabled) override;

		[[nodiscard]] bool isVSync() const override;
		void init(const WindowAttributes& data = WindowAttributes()) override;
	private:
		virtual void shutdown();

	private:

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		GLFWwindow* window;
		GraphicsContext* _context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			bool windowResized;
			EventCallbackFn EventCallback;
		};

		WindowData data;
	};
}
