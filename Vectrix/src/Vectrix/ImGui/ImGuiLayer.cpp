#include "vcpch.h"
#include "ImGuiLayer.h"

#include "Vectrix/Application.h"
#include "Vectrix/Debug/Profiler.h"

namespace Vectrix {
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer"), m_manager(ImGuiManager::create(Application::instance().window())), m_attached(false) {

	}

	ImGuiLayer::~ImGuiLayer() {
		VC_PROFILER_FUNCTION();
		VC_CORE_INFO("Destroying ImGuiLayer");
		if (m_attached)
			m_manager->cleanup();
		m_manager.reset();
		VC_CORE_INFO("ImGui destroyed");
	}

	void ImGuiLayer::OnRender() {
		VC_PROFILER_FUNCTION();
		m_manager->render();
	}

	void ImGuiLayer::OnEvent(Event &event) {
		if (m_blockEvents)	{
			ImGuiIO& io = ImGui::GetIO();
			event.Handled |= event.isInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			event.Handled |= event.isInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::OnUpdate(const DeltaTime& deltaTime) {
		VC_PROFILER_FUNCTION();
		m_manager->update();
		const ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::OnAttach() {
		VC_PROFILER_FUNCTION();
		m_manager->initImGui();
		m_attached = true;
	}

	void ImGuiLayer::OnDetach() {
		VC_PROFILER_FUNCTION();
		m_manager->cleanup();
		m_attached = false;
	}

	void ImGuiLayer::OnImGuiRender() {
		VC_PROFILER_FUNCTION();
		for (auto* w : m_widgets) {
			if (w != nullptr && w->isEnable())
				w->render();
		}
	}

	void ImGuiLayer::removeWidget(ImGuiWidget* widget) {
		const auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
		if (it != m_widgets.end())
			m_widgets.erase(it);
	}

	void ImGuiLayer::setDarkThemeColors() {
		auto& colors = ImGui::GetStyle().Colors;
		// Headers
		colors[ImGuiCol_Header] = {0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_HeaderHovered] = { 0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_HeaderActive] = { 0.15f, 0.1505f, 0.151f, 1.0f};

		// Buttons
		colors[ImGuiCol_Button] = { 0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_ButtonHovered] = { 0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_ButtonActive] = { 0.15f, 0.1505f, 0.151f, 1.0f};

		// Frame BG
		colors[ImGuiCol_FrameBg] = { 0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_FrameBgHovered] = { 0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_FrameBgActive] = { 0.15f, 0.1505f, 0.151f, 1.0f};

		// Tabs
		colors[ImGuiCol_Tab] = { 0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabHovered] = { 0.38f, 0.3805f, 0.381f, 1.0f};
		colors[ImGuiCol_TabActive] = { 0.28f, 0.2805f, 0.281f, 1.0f};
		colors[ImGuiCol_TabUnfocused] = { 0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabUnfocusedActive] = { 0.2f, 0.205f, 0.21f, 1.0f};

		// Title
		colors[ImGuiCol_TitleBg] = { 0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgActive] = { 0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgCollapsed] = { 0.15f, 0.1505f, 0.151f, 1.0f};


		colors[ImGuiCol_WindowBg] = {0.10f, 0.105f, 0.11f, 1.0f};
		colors[ImGuiCol_ChildBg] = {0.10f, 0.105f, 0.11f, 1.0f};
		colors[ImGuiCol_PopupBg] = {0.10f, 0.105f, 0.11f, 1.0f};
		colors[ImGuiCol_DockingEmptyBg]= {0.10f, 0.105f, 0.11f, 1.0f};
	}
}
