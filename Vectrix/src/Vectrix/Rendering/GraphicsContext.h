#pragma once
#include <GLFW/glfw3.h>

namespace Vectrix {
	/// @cond INTERNAL
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
	private:
		virtual void swapBuffers() = 0;
		virtual void init() = 0;
		static void setClientAPI();
		friend class Window;
		static GraphicsContext *create(GLFWwindow *windowHandle);
	};
	///  @endcond
}
