#include "vcpch.h"
#include "ImGuiLayer.h"
#include "backends/imgui_impl_glfw.h"

#include "Vectrix/Application.h"

namespace Vectrix {
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer"), m_manager(ImGuiManager::create(Application::instance().window()))
	{

	}

	ImGuiLayer::~ImGuiLayer() = default;

	void ImGuiLayer::OnRender() {
		m_manager->render();
	}

	void ImGuiLayer::OnUpdate(DeltaTime deltaTime) {
		m_manager->update();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::OnAttach()
	{
		m_manager->initImGui();
		m_manager->attachDebugGraphicWidget();
	}

	void ImGuiLayer::OnDetach()
	{
		m_manager->cleanup();
	}

	void ImGuiLayer::OnImGuiRender() {
		for (const auto& w : m_widgets) {
			w->render();
		}
	}
}