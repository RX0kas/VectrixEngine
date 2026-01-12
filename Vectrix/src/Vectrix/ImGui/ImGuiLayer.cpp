#include "vcpch.h"
#include "ImGuiLayer.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "Vectrix/Application.h"

namespace Vectrix {
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer"), manager(Application::instance().window(), Application::device())
	{

	}

	ImGuiLayer::~ImGuiLayer() = default;

	void ImGuiLayer::OnUpdate(DeltaTime deltaTime) {
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::OnAttach()
	{
		manager.initImGui();
	}

	void ImGuiLayer::OnDetach()
	{
		manager.cleanup();
	}

	void ImGuiLayer::OnImGuiRender() {

	}
}