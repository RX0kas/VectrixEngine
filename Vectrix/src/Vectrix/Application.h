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
#include "Core/FixedTimestep.h"
#include "Threading/JobSystem.h"
#include "Utils/Json.h"

#include <functional>
#include <mutex>
#include <vector>


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

/**
 * @brief Declare the name of the application, its version coming from its CMake project()
 *
 * Same as VC_SET_APP_INFO, without writing the version a second time: the target's CMakeLists.txt calls
 * `vectrix_set_app_version(<target>)`, which passes the `VERSION` of its `project()` as VC_APP_VERSION_MAJOR,
 * VC_APP_VERSION_MINOR and VC_APP_VERSION_PATCH.
 * @code
 * # CMakeLists.txt
 * project(MyGame VERSION 0.1.0)
 * add_executable(MyGame ...)
 * target_link_libraries(MyGame PRIVATE Vectrix)
 * vectrix_set_app_version(MyGame)
 * @endcode
 * @code
 * // MyGame.cpp
 * VC_SET_APP_INFO_FROM_PROJECT("MyGame")
 * @endcode
 * @param name The name of the application
 * @note Without vectrix_set_app_version on the target, it doesn't compile (static_assert)
 * @see VC_SET_APP_INFO
 * @ingroup core
 */
#if defined(VC_APP_VERSION_MAJOR) && defined(VC_APP_VERSION_MINOR) && defined(VC_APP_VERSION_PATCH)
	#define VC_SET_APP_INFO_FROM_PROJECT(name) VC_SET_APP_INFO(name, VC_APP_VERSION_MAJOR, VC_APP_VERSION_MINOR, VC_APP_VERSION_PATCH)
#else
	#define VC_SET_APP_INFO_FROM_PROJECT(name) static_assert(false, "VC_SET_APP_INFO_FROM_PROJECT needs vectrix_set_app_version(<target>) in the target's CMakeLists.txt");
#endif

int main(int argc, char** argv);

namespace Vectrix {
	class SettingsManager;

	/// @cond INTERNAL
	namespace detail {
		// What Application::observeEvents's observers are notified with, after each event was sent
		class EventDispatchedEvent : public EventBase<EventDispatchedEvent, "EventDispatched"> {
		public:
			EventDispatchedEvent(const Event& event, std::string_view consumedBy) : event(event), consumedBy(consumedBy) {}
			const Event& event;
			std::string_view consumedBy;
		};
	}
	/// @endcond

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
		 * @note Can be called from any thread (e.g. an asset loading thread): the event is still sent on the
		 *       main thread. The events of other threads come after the main thread's in the frame
		 * @see sendEvent
		 * @see EventListener::subscribe
		 */
		template<std::derived_from<Event> T, typename... Args> requires std::constructible_from<T, Args...>
		void postEvent(Args&&... args) { m_eventQueue.post<T>(std::forward<Args>(args)...); }

		/**
		 * @brief Send an event to the listeners right away, before returning
		 *
		 * For when the answer is needed now, e.g. to ask whether a listener objects before doing something.
		 * The event goes through the same path as a posted one (ImGui, the layers from the top, then the
		 * application), so an unconsumed WindowCloseEvent closes the application.
		 * @tparam T The event class
		 * @param args The arguments of T's constructor
		 * @return Whether a listener consumed the event
		 * @note Main thread only. Prefer postEvent unless the answer matters: sending from inside a handler
		 *       nests the dispatch
		 * @see postEvent
		 */
		template<std::derived_from<Event> T, typename... Args> requires std::constructible_from<T, Args...>
		bool sendEvent(Args&&... args) {
			T event(std::forward<Args>(args)...);
			dispatchEvent(event);
			return event.isHandled();
		}

		/**
		 * @brief Call a function after each event was sent, whether a listener consumed it or not
		 *
		 * Meant for tools such as an event log: unlike a subscription, an observer sees every event, those
		 * ImGui or a layer consumed included, and is told who consumed it. It can't consume nor change it.
		 * @param observer Called with the event and the name of the listener that consumed it: "ImGui", a
		 *                 layer's name or "Application", empty when none did
		 * @return The handle keeping the observer: it stops being called when the handle is destroyed
		 * @note Main thread only, like the events. An observer may post events, which are only sent next frame
		 */
		template<std::invocable<const Event&, std::string_view> F>
		[[nodiscard]] ScopedSubscription observeEvents(F&& observer) {
			return m_eventObservers.subscribeScoped<detail::EventDispatchedEvent>(
				[o = std::forward<F>(observer)](const detail::EventDispatchedEvent& e) mutable { std::invoke(o, e.event, e.consumedBy); });
		}

		/**
		 * @brief Run a function on the main thread, at the start of the next frame (before the events are sent)
		 *
		 * For a worker or a job whose result has to reach what is main thread only (ImGui, the scene, the
		 * assets managers...). The functions run in the order they were given, and the loop is woken if it waits.
		 * @param function What to run. It must not use what may be gone by the next frame (capture by value,
		 *                 or a weak_ptr)
		 * @note Any thread. From the main thread too: it then runs next frame, not now
		 */
		void runOnMainThread(std::function<void()> function);

		/**
		 * @brief Wake the main loop if it is waiting for events (e.g. while the window is minimized)
		 * @note Any thread
		 */
		void wakeUp() const;

		/**
		 * @brief The engine's pool of worker threads
		 * @see JobSystem
		 */
		[[nodiscard]] JobSystem& jobSystem() const { return *m_jobSystem; }

		/**
		 * @brief The time between two Layer::OnFixedUpdate calls, in seconds (`engine.physics.fixedTimestep`)
		 */
		[[nodiscard]] double getFixedTimestep() const { return m_fixedStep.step(); }

		/**
		 * @brief How far the frame being drawn is between the last fixed step and the next one, from 0 to 1
		 *
		 * To draw something the fixed steps move without it stuttering: show
		 * `mix(previousState, currentState, alpha)`.
		 */
		[[nodiscard]] double getFixedStepAlpha() const { return m_fixedStep.alpha(); }

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
		friend class Window;
		friend int ::main(int argc, char** argv);
		void renderImGui();
		void run();
		/// Sends one event down the layer stack, then to the application's own handlers
		void dispatchEvent(Event& event);

		bool m_hasToSwitch = false;
		std::shared_ptr<Layer> m_nextLayer;
		Layer* m_oldLayer = nullptr; ///< The layer switchToLayer replaces, only compared (it's still in the stack until then)
		JsonObject m_dataToNextLayer;


		/// First: destroyed after everything else, which may still wait for jobs while being destroyed
		std::unique_ptr<JobSystem> m_jobSystem;
		/// See runOnMainThread
		std::mutex m_mainThreadTasksMutex;
		std::vector<std::function<void()>> m_mainThreadTasks;
		std::vector<std::function<void()>> m_mainThreadTasksRunning; ///< Main thread only
		void runMainThreadTasks();

		FixedTimestep m_fixedStep; ///< engine.physics.fixedTimestep / maxStepsPerFrame
		/// Run the fixed steps the frame's time adds up to
		void runFixedSteps();

		EventListener m_eventObservers; ///< See observeEvents; notified apart, after each event
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
