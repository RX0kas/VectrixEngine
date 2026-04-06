#pragma once

#include "Core/Window.h"
#include "Core/AppInfo.h"
#include "Core/Core.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Events/Event.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "Rendering/Mesh/MeshManager.h"

using AppInfoFunc = Vectrix::ApplicationInfo(*)();
extern AppInfoFunc g_getAppInfo;

#define VC_SET_APP_INFO(name,major,minor,patch) AppInfoFunc g_getAppInfo = []() {return Vectrix::ApplicationInfo(name, major, minor, patch);};

int main(int argc, char** argv);

namespace Vectrix {
	class ShaderManager;
	class TextureManager;

	class Application {
	public:
		Application();
		virtual ~Application();

		void onEvent(Event& e);
		bool onWindowClose(WindowCloseEvent& e);

		void PushLayer(const std::shared_ptr<Layer>&  layer);
		void PushOverlay(const std::shared_ptr<Layer>&  layer);

		[[nodiscard]] Window &window() const { return *m_window; }
		[[nodiscard]] ImGuiLayer &imguiLayer() const { return *m_imGuiLayer; }
		[[nodiscard]] DeltaTime getDeltaTime() const { return m_deltaTime; }

		static Application& instance() { return *s_instance; }
		static ApplicationInfo getAppInfo() {
			if (g_getAppInfo) {
				return g_getAppInfo();
			}
			VC_CORE_ERROR("No information has been set for the application");
			return {"not_found",0};
		}

		void renderImGui();
	private:
		friend int ::main(int argc, char** argv);
		void run();
		std::unique_ptr<Window> m_window;
		std::unique_ptr<ShaderManager> m_shaderManager;
		std::unique_ptr<TextureManager> m_textureManager;
		std::unique_ptr<MeshManager> m_meshManager;
		std::unique_ptr<ImGuiLayer> m_imGuiLayer;
		std::unique_ptr<ApplicationInfo> m_appInfo;
		bool m_running = true;

		LayerStack m_layerStack;
		float m_LastFrameTime = 0.0f;
		DeltaTime m_deltaTime;

		static Application* s_instance;
	};

	Application* createApplication();

}