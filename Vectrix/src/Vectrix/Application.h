#pragma once

#include "Assets/AssetsManager.h"
#include "Core/Window.h"
#include "Core/AppInfo.h"
#include "Core/Core.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Vectrix/Events/EventListener.h"
#include "Vectrix/Events/EventQueue.h"
#include "Vectrix/Events/KeyEvent.h"
#include "Vectrix/Events/MouseEvent.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "Utils/Json.h"


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
	 *
	 * It is also an EventListener, the last one to receive each event (after every layer): its handlers
	 * only get what no layer consumed.
	 * @see createApplication
	 * @see Layer
	 * @ingroup core
	 */
	class Application : public EventListener {
	public:
		/**
		 * @brief Create the window, the assets manager and the ImGui overlay
		 * @note The window stays hidden until the application starts running
		 */
		Application();
		virtual ~Application();

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
		 * @tparam T New layer class
		 */
		template<std::derived_from<Layer> T>
		void switchToLayer(Layer* oldLayer, const JsonObject& data = JsonObject()) {
			m_nextLayer = std::make_shared<T>();
			m_oldLayer = oldLayer;
			if (!data.empty()) m_dataToNextLayer = data;

			m_hasToSwitch = true;
		}

		/**
		 * @brief Post an event, sent to the listeners at the start of the next frame
		 *
		 * Works for the engine's events and for the application's own (see EventBase).
		 * @tparam T The event class
		 * @param args The arguments of T's constructor
		 * @note Main thread only
		 * @see EventListener::subscribe
		 */
		template<std::derived_from<Event> T, typename... Args> requires std::constructible_from<T, Args...>
		void postEvent(Args&&... args) { m_eventQueue.post<T>(std::forward<Args>(args)...); }

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
		 * @note Unconditional, unlike the window's close button whose WindowCloseEvent a listener can cancel
		 *       by consuming it
		 **/
		void close() {
			m_running = false;
		}

		static SettingsManager& getSettingsManager() {
			VC_CORE_ASSERT(s_instance, "Vectrix has not been created");
			return *s_instance->m_settingsManager;
		}

		template<std::derived_from<Layer> T>
		void PushLayer() { PushLayer(std::make_shared<T>()); }

		template<std::derived_from<Layer> T>
		void PushOverlay() { PushOverlay(std::make_shared<T>()); }
	private:
		friend class VulkanImGuiManager;
		friend int ::main(int argc, char** argv);
		void renderImGui();
		void run();
		/// Sends one event down the layer stack, then to the application's own handlers
		void dispatchEvent(Event& event);

		bool m_hasToSwitch = false;
		std::shared_ptr<Layer> m_nextLayer;
		Layer* m_oldLayer = nullptr; ///< The layer switchToLayer replaces, only compared (it's still in the stack until then)
		JsonObject m_dataToNextLayer;


		EventQueue m_eventQueue; ///< Before m_window, which posts to it, so it is destroyed after
		std::unique_ptr<Window> m_window;
		std::unique_ptr<AssetsManager> m_assetsManager;
		std::unique_ptr<ImGuiLayer> m_imGuiLayer;
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
	 * @param argc The number of command line arguments
	 * @param argv The command line arguments, e.g. a project/scene file path passed by
	 *             the OS when the application is launched via a file association. In UTF-8 on
	 *             every platform: turn a path into a std::filesystem::path with fromUtf8
	 * @return The application, which the engine takes ownership of
	 * @see EntryPoint.h
	 * @ingroup core
	 */
	Application* createApplication(int argc, char** argv);

}
