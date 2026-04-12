#include "vcpch.h"
#include "ImGuiLayer.h"
#include "backends/imgui_impl_glfw.h"

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
	};

	void ImGuiLayer::OnRender() {
		VC_PROFILER_FUNCTION();
		m_manager->render();
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
#ifdef VC_DEBUG
		m_manager->attachDebugGraphicWidget();
#endif
		m_attached = true;
	}

	void ImGuiLayer::OnDetach() {
		VC_PROFILER_FUNCTION();
		m_manager->cleanup();
		m_attached = false;
	}

	void ImGuiLayer::OnImGuiRender() {
		VC_PROFILER_FUNCTION();
		for (const auto& w : m_widgets) {
			w->render();
		}
	}
}
