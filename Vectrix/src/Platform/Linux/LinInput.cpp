#include "vcpch.h"
#include "LinInput.h"

#include "Vectrix/Application.h"

namespace Vectrix {
	Input* Input::s_instance = new LinInput();

	bool LinInput::isKeyPressedImpl(int keycode)
	{
		const auto window = static_cast<GLFWwindow*>(Application::instance().window().getNativeWindow());
		const auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool LinInput::isMouseButtonPressedImpl(int button)
	{
		const auto window = static_cast<GLFWwindow*>(Application::instance().window().getNativeWindow());
		const auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> LinInput::getMousePositionImpl()
	{
		const auto window = static_cast<GLFWwindow*>(Application::instance().window().getNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { static_cast<float>(xpos), static_cast<float>(ypos) };
	}

	float LinInput::getMouseXImpl()
	{
		auto [x, y] = getMousePositionImpl();
		return x;
	}

	float LinInput::getMouseYImpl()
	{
		auto [x, y] = getMousePositionImpl();
		return y;
	}
}