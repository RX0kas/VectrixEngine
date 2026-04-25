#pragma once

#include "GLFW/glfw3.h"
#include "Vectrix/Scene/Components/MeshComponent.h"

namespace Vectrix {
	/// @cond INTERNAL
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;
		virtual void registerMesh(MeshComponent* model) = 0;
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
