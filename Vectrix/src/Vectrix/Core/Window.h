#pragma once

#include <utility>

#include "vcpch.h"

#include "Core.h"
#include "Vectrix/Events/Event.h"
#include "Vectrix/Rendering/GraphicsContext.h"

namespace Vectrix {
	struct WindowAttributes
	{
		std::string title;
		unsigned int width;
		unsigned int height;

		WindowAttributes(std::string  title = "Vectrix Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: title(std::move(title)), width(width), height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Vectrix_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		void init(const WindowAttributes& data = WindowAttributes());
		~Window();

		void onUpdate();

		[[nodiscard]] unsigned int getWidth() const { return m_data.Width; }
		[[nodiscard]] unsigned int getHeight() const { return m_data.Height;}

		[[nodiscard]] float getAspect() const {
			return m_context->getAspect();
		}

		// Window attributes
		[[nodiscard]] bool wasWindowResized() const { return m_data.windowResized; }

		void resetWindowResizedFlag() { m_data.windowResized = false; }

		[[nodiscard]] void* getNativeWindow() const { return m_window; }

		void setEventCallback(const EventCallbackFn& callback) { m_data.EventCallback = callback; }

		void setVSync(bool enabled);
		[[nodiscard]] bool isVSync() const;

		void show() {
			glfwShowWindow(m_window);
			m_data.visible = true;
		}

		void hide() {
			glfwHideWindow(m_window);
			m_data.visible = false;
		}

		[[nodiscard]] bool isVisible() const {
			return m_data.visible;
		}
	private:
		Window();

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			bool windowResized;
			EventCallbackFn EventCallback;
			bool visible;
		};
		
		void shutdown();

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		GLFWwindow* m_window;
		Own<GraphicsContext> m_context;

		WindowData m_data;
		
		static GraphicsContext* createGraphicContext(GLFWwindow* window) {
			GraphicsContext* g = GraphicsContext::create(window);
			g->init();
			return g;
		}
		static void setClientAPI() {GraphicsContext::setClientAPI();}
		friend class Application;
		static Window* create();
	};
}
