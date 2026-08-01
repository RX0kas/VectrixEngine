#pragma once

#include <utility>

#include "vcpch.h"

#include "Vectrix/Events/Event.h"

namespace Vectrix {
	enum DisplayServer {
		UNKNOWN_DISPLAY_SERVER,
		X11,
		WAYLAND,
		WINDOWS,
	};

	struct WindowAttributes
	{
		unsigned int width;
		unsigned int height;

		WindowAttributes( unsigned int width = 1280, unsigned int height = 720) : width(width), height(height) {}
	};

	class GraphicsContext;

	class Window {
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		void init(const WindowAttributes& data = WindowAttributes());
		~Window();

		void onUpdate() const;

		[[nodiscard]] unsigned int getWidth() const { return m_data.width; }
		[[nodiscard]] unsigned int getHeight() const { return m_data.height;}

		[[nodiscard]] float getAspect() const;

		// Window attributes
		[[nodiscard]] bool wasWindowResized() const { return m_data.windowResized; }

		void resetWindowResizedFlag() { m_data.windowResized = false; }

		[[nodiscard]] void* getNativeWindow() const { return m_window; }
		[[nodiscard]] GraphicsContext& getGraphicContext() const { return *m_context;}

		void setEventCallback(const EventCallbackFn& callback) { m_data.eventCallback = callback; }

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

		void setTitle(const std::string &title);
		[[nodiscard]] std::string getTitle() const { return m_data.title; }

		[[nodiscard]] DisplayServer getDisplayServer() const { return m_data.displayServer; }
	private:
		Window();

		struct WindowData
		{
			std::string title;
			unsigned int width, height;
			bool vSync;
			bool windowResized;
			EventCallbackFn eventCallback;
			bool visible;
			DisplayServer displayServer;
		};
		
		void shutdown();

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		GLFWwindow* m_window;
		std::unique_ptr<GraphicsContext> m_context;

		WindowData m_data;

		static DisplayServer detectLinuxDisplayServer();
		
		static GraphicsContext* createGraphicContext(GLFWwindow* window);
		static void setClientAPI();
		friend class Application;
		static Window* create();
	};
}
