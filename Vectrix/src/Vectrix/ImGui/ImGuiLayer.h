#pragma once
#include "ImGuiManager.h"
#include "ImGuiWidget.h"
#include "Vectrix/Layers/Layer.h"

/**
 * @file ImGuiLayer.h
 * @brief Definition of the ImGuiLayer
 * @ingroup imgui
 */

namespace Vectrix {
	/**
	 * @brief This function is the layer responsible for drawing Dear ImGui
	 *
	 * The Application creates it and keeps it as an overlay, so it draws on top of
	 * everything and gets the events first: the mouse and keyboard events ImGui wants
	 * don't reach the layers below (see startBlockEvents). Widgets registered through addWidget are
	 * rendered by this layer every frame.
	 * @see ImGuiWidget
	 * @ingroup imgui
	 */
	class ImGuiLayer : public Layer	{
	public:
		~ImGuiLayer() override;
		/// @cond INTERNAL
		void OnRender() override;
		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;
		void OnUpdate(const DeltaTime& deltaTime) override;
		/// @endcond

		/**
		 * @brief This function add a new ImGuiWidget
		 * @param widget A shared_ptr to the widget
		 */
		void addWidget(ImGuiWidget* widget) {
			m_widgets.push_back(widget);
		}
		/**
		 * @brief Stop drawing a widget that was added before
		 * @param widget The widget to remove
		 * @note Does nothing when the widget was never added
		 */
		void removeWidget(ImGuiWidget* widget);

		/**
		 * @brief Keep the events handled by ImGui from reaching the layers below
		 * @see stopBlockEvents
		 */
		void startBlockEvents() { m_blockEvents = true; }

		/**
		 * @brief Let every event through, even the ones ImGui handled
		 * @see startBlockEvents
		 */
		void stopBlockEvents() { m_blockEvents = false; }

		/**
		 * @brief Show or hide the graphics debug window (GPU, swapchain, textures, attached capture tool...)
		 *
		 * The engine draws it itself, on top of the application's ImGui, while it is visible; its close button
		 * hides it. F3 also toggles it, unless setGraphicsDebugShortcut(false) was called.
		 * @param visible Whether to show it
		 */
		void setGraphicsDebugVisible(bool visible) { m_graphicsDebugVisible = visible; }
		/**
		 * @brief Whether the graphics debug window is shown
		 * @see setGraphicsDebugVisible
		 */
		[[nodiscard]] bool isGraphicsDebugVisible() const { return m_graphicsDebugVisible; }
		/**
		 * @brief Let F3 toggle the graphics debug window or not (on by default)
		 * @param enabled false when the application uses F3 for something else
		 * @note F3 is ignored while an ImGui text field is being typed in
		 */
		void setGraphicsDebugShortcut(bool enabled) { m_graphicsDebugShortcut = enabled; }
	private:
		friend class Application;
		friend class EditorLayer;
		friend class VulkanImGuiManager;
		[[nodiscard]] ImGuiManager& getManager() const { return *m_manager; }
		/// Whether ImGui wants the event for itself (mouse over a window, text field focused...), which then
		/// doesn't reach the layers below
		[[nodiscard]] bool capturesEvent(const Event& event) const;
		ImGuiLayer();
		static void setDarkThemeColors();
		std::unique_ptr<ImGuiManager> m_manager;
		std::vector<ImGuiWidget*> m_widgets;
		/// Write ImGui's settings (imgui.ini) now, and not again when ImGui is destroyed: the Application calls it
		/// before detaching the layers, whose ImGui settings handlers would be gone by then
		void saveSettings();
		bool m_attached;
		bool m_blockEvents = true;
		bool m_graphicsDebugVisible = false;
		bool m_graphicsDebugShortcut = true;
	};

}
