#pragma once
#include <string>

#include "GLFW/glfw3.h"
#include "Vectrix/Core/Core.h"

namespace Vectrix {
	/// @cond INTERNAL
	class Model;
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
		virtual void registerMesh(const std::string& name,Ref<Model> model) = 0;
	private:
		friend class Application;
		static void uploadAllMeshData();
		virtual float getAspect() = 0;
		virtual void swapBuffers() = 0;
		virtual void init() = 0;
		static void setClientAPI();
		friend class Window;
		static GraphicsContext *create(GLFWwindow *windowHandle);
	};
	///  @endcond
}
