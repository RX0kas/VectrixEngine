#pragma once

#include "Core/Window.h"
#include "Core/AppInfo.h"
#include "Core/Core.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Events/Event.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "Rendering/Mesh/MeshManager.h"


/**
 * @file Application.h
 * @brief Definition of the main Application class
 * @ingroup core
 */

/// @cond INTERNAL
using AppInfoFunc = Vectrix::ApplicationInfo(*)();
extern AppInfoFunc g_getAppInfo;
/// @endcond

#define VC_SET_APP_INFO(name,major,minor,patch) AppInfoFunc g_getAppInfo = []() {return Vectrix::ApplicationInfo(name, major, minor, patch);};

int main(int argc, char** argv);

namespace Vectrix {
	class ShaderManager;
	class TextureManager;

	class Application {
	public:
		Application();
		virtual ~Application();

		/// @cond INTERNAL
		void onEvent(Event& e);
		bool onWindowClose(WindowCloseEvent& e);
		/// @endcond

		/**
		 * @brief This function add a layer that will be rendered
		 * @param layer The custom layer you wanna add
		 **/
		void PushLayer(const std::shared_ptr<Layer>& layer);
		/**
		 * @brief This function add a layer that will be rendered, it will render on top of the other layer
		 * @param layer The custom layer you wanna add
		 **/
		void PushOverlay(const std::shared_ptr<Layer>& layer);

		/**
		 * @brief This function return the current Window instance
		 */
		[[nodiscard]] Window &window() const { return *m_window; }
		/**
		 * @brief This function return the current ImGuiLayer instance
		 */
		[[nodiscard]] ImGuiLayer &imguiLayer() const { return *m_imGuiLayer; }
		/**
		 * @brief This function return the current DeltaTime
		 */
		[[nodiscard]] DeltaTime getDeltaTime() const { return m_deltaTime; }

		/**
		 * @brief This function return the instance of the application
		 */
		static Application& instance() { return *s_instance; }

		/**
		 * @brief This function return the information about the current application
		 */
		static ApplicationInfo getAppInfo() {
			if (g_getAppInfo) {
				return g_getAppInfo();
			}
			VC_CORE_ERROR("No information has been set for the application");
			return {"not_found",0};
		}

	private:
		friend class ImGuiManager;
		friend int ::main(int argc, char** argv);
		void renderImGui();
		void run();
		std::unique_ptr<Window> m_window;
		std::unique_ptr<ShaderManager> m_shaderManager;
		std::unique_ptr<TextureManager> m_textureManager;
		std::unique_ptr<MeshManager> m_meshManager;
		std::unique_ptr<ImGuiLayer> m_imGuiLayer;
		std::unique_ptr<ApplicationInfo> m_appInfo;
		bool m_running = true;

		LayerStack m_layerStack;
		float m_LastFrameTime = 0.0f;
		DeltaTime m_deltaTime;

		static Application* s_instance;
	};

	Application* createApplication();

}