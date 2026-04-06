#include "Window.h"

#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Events/KeyEvent.h"
#include "Vectrix/Events/MouseEvent.h"
#include "Vectrix/Events/WindowEvent.h"
#include "Vectrix/Rendering/GraphicsContext.h"

namespace Vectrix {
	static uint8_t s_GLFWWindowCount = 0;

	static void errorCallback(int error, const char* description) {
		VC_CORE_CRITICAL("GLFW Error ({0}): {1}", error, description);
	}

	void Window::shutdown() {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Destroying Window");
		glfwDestroyWindow(m_window);

		if (s_GLFWWindowCount-- == 0) {
			VC_CORE_INFO("Terminating GLFW");
			glfwTerminate();
		}

		m_context.reset();
	}

	Window::Window() : m_window(nullptr), m_data() {
		VC_PROFILER_FUNCTION();
		if (!s_GLFWWindowCount) {
			VC_CORE_INFO("Initializing GLFW");
			int success = glfwInit();
			VC_CORE_ASSERT(success, "Could not intialize GLFW!");

			glfwSetErrorCallback(errorCallback);

			setClientAPI();
		}
		s_GLFWWindowCount++;
	}

	Window::~Window() {
		VC_PROFILER_FUNCTION();
		shutdown();
	}

	void Window::init(const WindowAttributes& attributes) {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Creating window {0} ({1}, {2})", attributes.title, attributes.width, attributes.height);

		m_data.Width = attributes.width;
		m_data.Height = attributes.height;
		m_data.Title = attributes.title;
		m_data.visible = false;

		glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
		m_window = glfwCreateWindow(static_cast<int>(attributes.width), static_cast<int>(attributes.height), attributes.title, nullptr, nullptr);
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

			switch (action) {
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
				default: {
					VC_CORE_CRITICAL("Unknown KeyButtonCallback Action: {}",action);
				}
			}
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

			switch (action) {
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
				default: {
					VC_CORE_CRITICAL("Unknown MouseButtonCallback Action: {}",action);
				}
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

			MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
			data.EventCallback(event);
		});


		m_context = std::unique_ptr<GraphicsContext>(createGraphicContext(m_window));
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
		data.Width = width;
		data.Height = height;
		data.windowResized = true;
		// TODO: implement that
	}

	Window* Window::create() {
		VC_PROFILER_FUNCTION();
		return new Window();
	}

	void Window::onUpdate() const {
		VC_PROFILER_FUNCTION();
		m_context->swapBuffers();
	}

	void Window::setVSync(bool enabled)	{
		VC_PROFILER_FUNCTION();
		// TODO: Changer la swapchain pour appliquer l'effet
		m_data.VSync = enabled;
	}

	bool Window::isVSync() const {
		return m_data.VSync;
	}

	GraphicsContext* Window::createGraphicContext(GLFWwindow* window) {
		GraphicsContext* g = GraphicsContext::create(window);
		g->init();
		return g;
	}
	void Window::setClientAPI() {
		GraphicsContext::setClientAPI();
	}

	float Window::getAspect() const {
		return m_context->getAspect();
	}
}
