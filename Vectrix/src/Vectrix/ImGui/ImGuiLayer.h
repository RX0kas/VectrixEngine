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
	 * everything and gets the events first. Widgets registered through addWidget are
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
		void OnEvent(Event &event) override;
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
	private:
		friend class Application;
		friend class EditorLayer;
		friend class VulkanImGuiManager;
		[[nodiscard]] ImGuiManager& getManager() const { return *m_manager; }
		ImGuiLayer();
		static void setDarkThemeColors();
		std::unique_ptr<ImGuiManager> m_manager;
		std::vector<ImGuiWidget*> m_widgets;
		bool m_attached;
		bool m_blockEvents = true;
	};

}
