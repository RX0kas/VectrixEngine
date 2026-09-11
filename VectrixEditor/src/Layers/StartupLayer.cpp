#include "StartupLayer.h"

#include "EditorLayer.h"
#include "imgui.h"
#include "Vectrix/Application.h"

namespace Vectrix {
	StartupLayer::StartupLayer() : Layer("StartupLayer") {

	}

	StartupLayer::~StartupLayer() {

	}


	void StartupLayer::OnAttach() {

	}

	void StartupLayer::OnImGuiRender() {
		ImGui::Begin("Test");
		ImGui::Text("Hey");
		if (ImGui::Button("Switch to Editor")) {
			Application::instance().switchToLayer<EditorLayer>(this);
		}
		ImGui::End();
	}
} // Vectrix