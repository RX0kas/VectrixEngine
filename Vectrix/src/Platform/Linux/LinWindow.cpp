#include "vcpch.h"
#include "LinWindow.h"

#include "Vectrix/Events/MouseEvent.h"
#include "Vectrix/Events/WindowEvent.h"
#include "Vectrix/Events/KeyEvent.h"

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Application.h"

namespace Vectrix {
	static bool s_GLFWInitialized = false; // Might be multiple window

	static void errorCallback(int error, const char* description)
	{
		VC_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	void LinWindow::shutdown() {
		glfwDestroyWindow(window);

		if (s_GLFWInitialized)
		{
			glfwTerminate();
			s_GLFWInitialized = false;
		}
	}

	LinWindow::LinWindow() {
		if (!s_GLFWInitialized) {
			VC_CORE_INFO("Initializing GLFW");
			int success = glfwInit();
			VC_CORE_ASSERT(success, "Could not intialize GLFW!");

			glfwSetErrorCallback(errorCallback);

			s_GLFWInitialized = true;


			// TODO: put it in a better place, like VulkanContext
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
	}

	void LinWindow::init(const WindowAttributes& attributes) {
		this->data.Width = attributes.width;
		this->data.Height = attributes.height;
		this->data.Title = attributes.title;

		VC_CORE_INFO("Creating window {0} ({1}, {2})", attributes.title, attributes.width, attributes.height);

		

		window = glfwCreateWindow(attributes.width, attributes.height, attributes.title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, &data);


		// Set some callbacks
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallback(event);
			});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

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

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
			{
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

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});


		_context = new VulkanContext(window);
		_context->init();
	}

	void LinWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		VC_CHECK_VK_SUCCESS(
		glfwCreateWindowSurface(instance, window, nullptr, surface),"Couldn't create a WindowSurface"
		)
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
		data.VSync = enabled;
	}

	bool LinWindow::isVSync() const
	{
		return data.VSync;
	}
}
