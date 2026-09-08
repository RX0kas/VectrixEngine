#pragma once

#include <utility>

#include "vcpch.h"

#include "Vectrix/Events/Event.h"

/**
 * @file Window.h
 * @brief Definition of the platform independent Window class
 * @ingroup core
 */

namespace Vectrix {
	/**
	 * @brief The windowing system the application is running on
	 *
	 * It is detected at startup and mostly matters on Linux, where the behaviour of the
	 * window differs between X11 and Wayland.
	 * @see Window::getDisplayServer
	 * @ingroup core
	 */
	enum DisplayServer {
		UNKNOWN_DISPLAY_SERVER, ///< The display server could not be identified
		X11,                    ///< X11, on Linux
		WAYLAND,                ///< Wayland, on Linux
		WINDOWS,                ///< The Windows desktop
	};

	/**
	 * @brief The properties used to create the window
	 * @see Window::init
	 * @ingroup core
	 */
	struct WindowAttributes
	{
		/// The width of the window in pixel
		unsigned int width;

		/// The height of the window in pixel
		unsigned int height;

		/**
		 * @brief Build the attributes, falling back on a 1280x720 window
		 * @param width The wanted width in pixel
		 * @param height The wanted height in pixel
		 */
		WindowAttributes( unsigned int width = 1280, unsigned int height = 720) : width(width), height(height) {}
	};

	class GraphicsContext;

	/**
	 * @brief The window the engine draws into, and the source of most events
	 *
	 * The Application owns the window and creates it, so an application should reach it
	 * through Application::getWindow rather than build one itself. It also owns the
	 * GraphicsContext bound to it.
	 * @see Application
	 * @see GraphicsContext
	 * @ingroup core
	 */
	class Window {
	public:
		/// The signature of the callback the window uses to report its events
		using EventCallbackFn = std::function<void(Event&)>;

		/**
		 * @brief Create the underlying window and its graphics context
		 * @param data The size the window should be created with
		 * @note The window stays hidden until show is called
		 * @see show
		 */
		void init(const WindowAttributes& data = WindowAttributes());
		~Window();

		/**
		 * @brief Process the pending window messages and present the frame
		 * @note Called once per frame by Application::run
		 */
		void onUpdate() const;

		/**
		 * @brief Return the current width of the window
		 * @return The width in pixel
		 */
		[[nodiscard]] unsigned int getWidth() const { return m_data.width; }

		/**
		 * @brief Return the current height of the window
		 * @return The height in pixel
		 */
		[[nodiscard]] unsigned int getHeight() const { return m_data.height;}

		/**
		 * @brief Return the aspect ratio of the window
		 * @return The width divided by the height
		 */
		[[nodiscard]] float getAspect() const;

		// Window attributes

		/**
		 * @brief Tell if the window changed size since the flag was last reset
		 * @return true when a resize happened
		 * @see resetWindowResizedFlag
		 */
		[[nodiscard]] bool wasWindowResized() const { return m_data.windowResized; }

		/**
		 * @brief Clear the resize flag, once the resize has been handled
		 * @see wasWindowResized
		 */
		void resetWindowResizedFlag() { m_data.windowResized = false; }

		/**
		 * @brief Return the handle of the window given by the platform
		 * @return The native handle, a `GLFWwindow*` as things stand
		 * @warning Only use it when a platform specific call really needs it
		 */
		[[nodiscard]] void* getNativeWindow() const { return m_window; }

		/**
		 * @brief Return the graphics context bound to this window
		 * @return A reference to the context, owned by the window
		 */
		[[nodiscard]] GraphicsContext& getGraphicContext() const { return *m_context;}

		/**
		 * @brief Set the function the window calls whenever an event happens
		 * @param callback The function receiving the events
		 * @note The Application sets it, so it can dispatch the events to the layer stack
		 */
		void setEventCallback(const EventCallbackFn& callback) { m_data.eventCallback = callback; }

		/**
		 * @brief Turn the vertical synchronisation on or off
		 * @param enabled true to wait for the vertical blank before presenting
		 * @note This recreates the swapchain
		 */
		void setVSync(bool enabled);

		/**
		 * @brief Tell if the vertical synchronisation is on
		 * @return true when vsync is enabled
		 */
		[[nodiscard]] bool isVSync() const;

		/**
		 * @brief Make the window visible
		 * @see hide
		 * @see isVisible
		 */
		void show() {
			glfwShowWindow(m_window);
			m_data.visible = true;
		}

		/**
		 * @brief Hide the window without destroying it
		 * @see show
		 */
		void hide() {
			glfwHideWindow(m_window);
			m_data.visible = false;
		}

		/**
		 * @brief Tell if the window is currently shown
		 * @return true when the window is visible
		 */
		[[nodiscard]] bool isVisible() const {
			return m_data.visible;
		}

		/**
		 * @brief Change the text shown in the title bar
		 * @param title The new title
		 */
		void setTitle(const std::string &title);

		/**
		 * @brief Return the text currently shown in the title bar
		 * @return The title of the window
		 */
		[[nodiscard]] std::string getTitle() const { return m_data.title; }

		/**
		 * @brief Return the windowing system the window runs on
		 * @return The detected display server
		 * @see DisplayServer
		 */
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
