#pragma once

#include "GLFW/glfw3.h"
#include "Vectrix/Scene/Components/MeshRendererComponent.h"

namespace Vectrix {
	/// @cond INTERNAL
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;
		virtual void registerMesh(Mesh* model) = 0;
		static void waitIdle();
	private:
		friend class Application;
		friend class EditorLayer;
		static void uploadAllMeshData();
		static void unloadGPUMeshData();
		virtual float getAspect() = 0;
		virtual void swapBuffers() = 0;
		virtual void init() = 0;
		static void setClientAPI();
		friend class Window;
		static GraphicsContext *create(GLFWwindow *windowHandle);
	};
	///  @endcond
}
