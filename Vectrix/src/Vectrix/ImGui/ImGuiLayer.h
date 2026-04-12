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
		void addWidget(const std::shared_ptr<ImGuiWidget> &widget) {
			m_widgets.push_back(widget);
		}
	private:
		friend class Application;
		ImGuiLayer();
		std::unique_ptr<ImGuiManager> m_manager;
		std::vector<std::shared_ptr<ImGuiWidget>> m_widgets;
		bool m_attached;
	};

}
