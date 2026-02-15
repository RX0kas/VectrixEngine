#pragma once

#include "Core/Core.h"
#include "Core/Window.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Events/Event.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "Renderer/Shaders/ShaderManager.h"

namespace Vectrix {
	class Vectrix_API Application
	{
	public:
		Application();
		virtual ~Application();
		void run();

		void onEvent(Event& e);
		bool onWindowClose(WindowCloseEvent& e);

		void PushLayer(const Ref<Layer>&  layer);
		void PushOverlay(const Ref<Layer>&  layer);

		[[nodiscard]] Window &window() const { return *m_window; }
		[[nodiscard]] ImGuiLayer &imguiLayer() const {return *m_ImGuiLayer;}
		[[nodiscard]] DeltaTime getDeltaTime() const {return m_deltaTime;}

		static Application& instance() { return *s_instance; }
		void renderImGui();
	private:
		Ref<Window> m_window;
		Ref<ShaderManager> m_shaderManager;
		Ref<ImGuiLayer> m_ImGuiLayer;
		bool m_running = true;

		LayerStack m_layerStack;
		float m_LastFrameTime = 0.0f;
		DeltaTime m_deltaTime;
	private:
		static Application* s_instance;
	};

	// TODO: to be defined in the client
	Application* createApplication();
}