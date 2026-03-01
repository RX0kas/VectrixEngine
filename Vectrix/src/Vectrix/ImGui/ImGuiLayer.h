#pragma once
#include "ImGuiManager.h"
#include "ImGuiWidget.h"
#include "Vectrix/Layers/Layer.h"

namespace Vectrix {

	class Vectrix_API ImGuiLayer : public Layer
	{
	public:
		~ImGuiLayer() override;

		void OnRender() override;
		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;
		void OnUpdate(const DeltaTime& deltaTime) override;
		void addWidget(const Ref<ImGuiWidget> &widget) {
			m_widgets.push_back(std::shared_ptr(widget));
		}
	private:
		friend class Application;
		ImGuiLayer();
		Own<ImGuiManager> m_manager;
		std::vector<Ref<ImGuiWidget>> m_widgets;
		bool m_attached;
	};

}
