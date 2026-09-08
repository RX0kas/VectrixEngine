#pragma once

#include "Assets/AssetsManager.h"
#include "Core/Window.h"
#include "Core/AppInfo.h"
#include "Core/Core.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Events/Event.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"


/**
 * @file Application.h
 * @brief Definition of the main Application class
 * @ingroup core
 */

/// @cond INTERNAL
using AppInfoFunc = Vectrix::ApplicationInfo(*)();
extern AppInfoFunc g_getAppInfo;
/// @endcond

/**
 * @brief Declare the name and the version of the application
 *
 * Put it once at file scope in the application, it is what Application::getAppInfo reads
 * from. Without it the engine has no name nor version to report.
 * @param name The name of the application
 * @param major The major part of its version
 * @param minor The minor part of its version
 * @param patch The patch part of its version
 * @see Vectrix::ApplicationInfo
 * @ingroup core
 */
#define VC_SET_APP_INFO(name,major,minor,patch) AppInfoFunc g_getAppInfo = []() {return Vectrix::ApplicationInfo(name, major, minor, patch);};

int main(int argc, char** argv);

namespace Vectrix {
	class SettingsManager;
	/**
	 * @brief The application itself, owning the window, the assets and the layers
	 *
	 * Derive from it, push the layers the application needs from the constructor, and
	 * return the instance from createApplication. The engine takes care of running it,
	 * so there is no main loop to write.
	 * @see createApplication
	 * @see Layer
	 * @ingroup core
	 */
	class Application {
	public:
		/**
		 * @brief Create the window, the assets manager and the ImGui overlay
		 * @note The window stays hidden until the application starts running
		 */
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

		/**
		 * @brief This function close the application
		 **/
		void close() {
			m_running = false;
		}

		static SettingsManager& getSettingsManager() {
			VC_CORE_ASSERT(s_instance, "Vectrix has not been created");
			return *s_instance->m_settingsManager;
		}
	private:
		friend class VulkanImGuiManager;
		friend int ::main(int argc, char** argv);
		void renderImGui();
		void run();


		std::unique_ptr<Window> m_window;
		std::unique_ptr<AssetsManager> m_assetsManager;
		std::unique_ptr<ImGuiLayer> m_imGuiLayer;
		std::unique_ptr<ApplicationInfo> m_appInfo;
		std::shared_ptr<SettingsManager> m_settingsManager;
		bool m_running = true;

		LayerStack m_layerStack;
		float m_LastFrameTime = 0.0f;
		DeltaTime m_deltaTime;

		static Application* s_instance;
	};

	/**
	 * @brief Build the application the engine should run
	 *
	 * The application has to define it, it is what the entry point calls to get the
	 * instance to run.
	 * @return The application, which the engine takes ownership of
	 * @see EntryPoint.h
	 * @ingroup core
	 */
	Application* createApplication();

}
