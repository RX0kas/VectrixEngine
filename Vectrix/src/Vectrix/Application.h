#pragma once

#include "Core/AppInfo.h"
#include "Core/Core.h"
#include "Core/Window.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Events/Event.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "Rendering/Shaders/ShaderManager.h"
#include "Rendering/Textures/TextureManager.h"

using AppInfoFunc = Vectrix::ApplicationInfo(*)();
extern AppInfoFunc g_getAppInfo;

#define VC_SET_APP_INFO(name,major,minor,patch) AppInfoFunc g_getAppInfo = []() {return Vectrix::ApplicationInfo(name, major, minor, patch);};

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
		[[nodiscard]] ImGuiLayer &imguiLayer() const {return *m_imGuiLayer;}
		[[nodiscard]] DeltaTime getDeltaTime() const {return m_deltaTime;}

		static Application& instance() { return *s_instance; }
		static ApplicationInfo getAppInfo() {
			if (g_getAppInfo) {
				return g_getAppInfo();
			}
			VC_CORE_ERROR("No information has been set for the application");
		}

		void renderImGui();
	private:
		Ref<Window> m_window;
		Ref<ShaderManager> m_shaderManager;
		Ref<TextureManager> m_textureManager;
		Ref<ImGuiLayer> m_imGuiLayer;
		Own<ApplicationInfo> m_appInfo;
		bool m_running = true;

		LayerStack m_layerStack;
		float m_LastFrameTime = 0.0f;
		DeltaTime m_deltaTime;
	private:
		static Application* s_instance;
	};

	Application* createApplication();

}