#include "vcpch.h"
#include "ImGuiLayer.h"
#include "backends/imgui_impl_glfw.h"

#include "Vectrix/Application.h"

namespace Vectrix {
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer"), m_manager(ImGuiManager::create(Application::instance().window())), m_attached(false) {
	}

	ImGuiLayer::~ImGuiLayer() {
		VC_CORE_INFO("Destroying ImGuiLayer");
		if (m_attached)
			m_manager->cleanup();

		auto* m = m_manager.release();
		delete m;
	};

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
		m_attached = true;
	}

	void ImGuiLayer::OnDetach()
	{
		m_manager->cleanup();
		m_attached = false;
	}

	void ImGuiLayer::OnImGuiRender() {
		for (const auto& w : m_widgets) {
			w->render();
		}
	}
}