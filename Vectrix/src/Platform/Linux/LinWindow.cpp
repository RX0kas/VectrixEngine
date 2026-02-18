#include "vcpch.h"
#include "LinWindow.h"

#include "Vectrix/Events/MouseEvent.h"
#include "Vectrix/Events/WindowEvent.h"
#include "Vectrix/Events/KeyEvent.h"

#include "Vectrix/Application.h"
#include "Vectrix/Renderer/GraphicsContext.h"

namespace Vectrix {
	static bool s_GLFWInitialized = false; // Might be multiple window

	static void errorCallback(int error, const char* description)
	{
		VC_CORE_CRITICAL("GLFW Error ({0}): {1}", error, description);
	}

	void LinWindow::shutdown() {
		VC_CORE_INFO("Destroying Window");
		glfwDestroyWindow(m_window);

		if (s_GLFWInitialized)
		{
			glfwTerminate();
			s_GLFWInitialized = false;
		}

		GraphicsContext* g = m_context.release();
		delete g;
	}

	LinWindow::LinWindow() {
		if (!s_GLFWInitialized) {
			VC_CORE_INFO("Initializing GLFW");
			int success = glfwInit();
			VC_CORE_ASSERT(success, "Could not intialize GLFW!");

			glfwSetErrorCallback(errorCallback);

			s_GLFWInitialized = true;

			GraphicsContext::setClientAPI();
		}
	}

	LinWindow::~LinWindow() {
		shutdown();
	}

	void LinWindow::init(const WindowAttributes& attributes) {
		VC_CORE_INFO("Creating window {0} ({1}, {2})", attributes.title, attributes.width, attributes.height);

		m_data.Width = attributes.width;
		m_data.Height = attributes.height;
		m_data.Title = attributes.title;

		m_window = glfwCreateWindow(attributes.width, attributes.height, attributes.title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, &m_data);


		// Set some callbacks
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
			WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, 1);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});


		m_context = std::unique_ptr<GraphicsContext>(GraphicsContext::create(m_window));
		m_context->init();
	}

	void LinWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		VC_VK_CHECK(glfwCreateWindowSurface(instance, m_window, nullptr, surface),"Couldn't create a WindowSurface")
	}

	void LinWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;
		data.windowResized = true;
		// TODO: implement that
	}

	Window* Window::create()
	{
		return new LinWindow();
	}

	void LinWindow::onUpdate()
	{
		VulkanContext::instance().swapBuffers();
	}

	void LinWindow::setVSync(bool enabled)
	{
		// TODO: Changer la swapchain pour appliquer l'effet
		m_data.VSync = enabled;
	}

	bool LinWindow::isVSync() const
	{
		return m_data.VSync;
	}
}
