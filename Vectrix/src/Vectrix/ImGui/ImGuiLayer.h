#pragma once
#include "ImGuiManager.h"
#include "ImGuiWidget.h"
#include "Vectrix/Layers/Layer.h"

namespace Vectrix {
	class ImGuiLayer : public Layer	{
	public:
		~ImGuiLayer() override;

		void OnRender() override;
		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;
		void OnUpdate(const DeltaTime& deltaTime) override;
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
