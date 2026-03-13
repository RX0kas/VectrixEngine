#include "vcpch.h"
#include "WinInput.h"

#include "Vectrix/Application.h"

namespace Vectrix {
	Input* Input::s_instance = new WinInput();

	bool WinInput::isKeyPressedImpl(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::instance().window().getNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WinInput::isMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::instance().window().getNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> WinInput::getMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::instance().window().getNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { static_cast<float>(xpos), static_cast<float>(ypos) };
	}

	float WinInput::getMouseXImpl()
	{
		auto [x, y] = getMousePositionImpl();
		return x;
	}

	float WinInput::getMouseYImpl()
	{
		auto [x, y] = getMousePositionImpl();
		return y;
	}
}