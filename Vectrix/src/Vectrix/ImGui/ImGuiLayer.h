#pragma once
#include "ImGuiManager.h"
#include "ImGuiWidget.h"
#include "Vectrix/Layers/Layer.h"

namespace Vectrix {

	class Vectrix_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;

		void OnRender() override;
		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;
		void OnUpdate(DeltaTime deltaTime) override;
		void addWidget(const Ref<ImGuiWidget> &widget) {
			m_widgets.push_back(std::shared_ptr(widget));
		}
	private:
		ImGuiManager *m_manager;
		std::vector<Ref<ImGuiWidget>> m_widgets;
	};

}
