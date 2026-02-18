#pragma once
#include <GLFW/glfw3.h>

namespace Vectrix {
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void init() = 0;
		virtual void swapBuffers() = 0;
		static GraphicsContext *create(GLFWwindow *windowHandle);
		static void setClientAPI();
	};
}
