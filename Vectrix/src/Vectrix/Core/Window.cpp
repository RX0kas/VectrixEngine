#include "Window.h"

#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Events/KeyEvent.h"
#include "Vectrix/Events/MouseEvent.h"
#include "Vectrix/Events/WindowEvent.h"
#include "Vectrix/Rendering/GraphicsContext.h"

namespace Vectrix {
	static uint8_t s_GLFWWindowCount = 0;


	/// @cond INTERNAL
	static void errorCallback(int error, const char* description) {
		VC_CORE_CRITICAL("GLFW Error ({0}): {1}", error, description);
	}
	/// @endcond

	void Window::shutdown() {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Destroying Window");
		// The graphics context owns the window's Vulkan surface, which has to go before the window itself
		m_context.reset();
		glfwDestroyWindow(m_window);

		if (--s_GLFWWindowCount == 0) {
			VC_CORE_INFO("Terminating GLFW");
			glfwTerminate();
		}
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

	template<std::derived_from<Event> T, typename... Args>
	void Window::postEvent(GLFWwindow* window, Args&&... args) {
		const auto* data = static_cast<const WindowData*>(glfwGetWindowUserPointer(window));
		if (data && data->eventQueue)
			data->eventQueue->post<T>(std::forward<Args>(args)...);
	}

	void Window::init(const WindowAttributes& attributes) {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Creating window {0} ({1}, {2})", Application::getAppInfo().getAppName(), attributes.width, attributes.height);

		m_data.title = Application::getAppInfo().getAppName();
		m_data.visible = false;
#ifdef VC_PLATFORM_WINDOWS
		m_data.displayServer = WINDOWS;
#else
		m_data.displayServer = detectLinuxDisplayServer();
#endif



		glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
		m_window = glfwCreateWindow(static_cast<int>(attributes.width), static_cast<int>(attributes.height), m_data.title.c_str(), nullptr, nullptr);
		if (m_window == nullptr) {
			VC_CORE_CRITICAL("Failed to create the GLFW window");
			return;
		}
		glfwSetWindowUserPointer(m_window, &m_data);

		// width/height track the framebuffer in pixels, which the swap chain has to match. On a scaled
		// display (HiDPI, Wayland scale factor) it differs from the window size in screen coordinates.
		int framebufferWidth = 0, framebufferHeight = 0;
		glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
		m_data.width = static_cast<unsigned int>(framebufferWidth);
		m_data.height = static_cast<unsigned int>(framebufferHeight);

		// Set some callbacks
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

		// Set before ImGui installs its own callbacks, which forward to these
		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
			postEvent<WindowResizeEvent>(window, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
			postEvent<WindowCloseEvent>(window);
		});

		glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focused) {
			postEvent<WindowFocusEvent>(window, focused == GLFW_TRUE);
		});

		glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int x, int y) {
			postEvent<WindowMovedEvent>(window, x, y);
		});

		glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* window, int iconified) {
			postEvent<WindowMinimizeEvent>(window, iconified == GLFW_TRUE);
		});

		glfwSetDropCallback(m_window, [](GLFWwindow* window, int count, const char** paths) {
			// GLFW gives the paths in UTF-8, and frees them once this returns
			postEvent<FilesDroppedEvent>(window, std::vector<std::string>(paths, paths + count));
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			switch (action) {
				case GLFW_PRESS:   postEvent<KeyPressedEvent>(window, key, false); break;
				case GLFW_REPEAT:  postEvent<KeyPressedEvent>(window, key, true); break;
				case GLFW_RELEASE: postEvent<KeyReleasedEvent>(window, key); break;
				default: VC_CORE_ERROR_NO_EXIT("Unknown key action: {}", action);
			}
		});

		glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int codepoint) {
			postEvent<KeyTypedEvent>(window, static_cast<char32_t>(codepoint));
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
			switch (action) {
				case GLFW_PRESS:   postEvent<MouseButtonPressedEvent>(window, button); break;
				case GLFW_RELEASE: postEvent<MouseButtonReleasedEvent>(window, button); break;
				default: VC_CORE_ERROR_NO_EXIT("Unknown mouse button action: {}", action);
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
			postEvent<MouseScrolledEvent>(window, static_cast<float>(xOffset), static_cast<float>(yOffset));
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
			postEvent<MouseMovedEvent>(window, static_cast<float>(xPos), static_cast<float>(yPos));
		});

		m_context = std::unique_ptr<GraphicsContext>(createGraphicContext(m_window));
	}

	void Window::setTitle(const std::string &title) {
		m_data.title = title; // so getTitle reports what is shown
		glfwSetWindowTitle(m_window,title.c_str());
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		WindowData& data = *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
		data.width = width;
		data.height = height;
		data.windowResized = true; // VulkanRenderer::endFrame recreates the swap chain on it
	}

	Window* Window::create() {
		VC_PROFILER_FUNCTION();
		return new Window();
	}

	void Window::onUpdate() const {
		VC_PROFILER_FUNCTION();
		m_context->swapBuffers();
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

	DisplayServer Window::detectLinuxDisplayServer() {
		const char* sessionType = std::getenv("XDG_SESSION_TYPE");

		if (sessionType != nullptr) {
			std::string type(sessionType);
			if (type == "wayland") return WAYLAND;

			if (type == "x11") return X11;

			VC_CORE_ERROR_NO_EXIT("Unknown XDG_SESSION_TYPE variable : {}", sessionType);
			return UNKNOWN_DISPLAY_SERVER;
		}

		VC_CORE_ERROR_NO_EXIT("Undefined XDG_SESSION_TYPE variable");
		return UNKNOWN_DISPLAY_SERVER;
	}
}
