#pragma once

#include "Core/Core.h"
#include "Window.h"
#include "GraphicAPI/Vulkan/rendering/Device.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Events/Event.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "Renderer/Camera/PerspectiveCamera.h"
#include "Renderer/Shaders/ShaderManager.h"
#include "Renderer/Models/VertexArray.h"

#include "Renderer/Shaders/Shader.h"
#include "Vectrix/Renderer/Buffer.h"

namespace Vectrix {
	// TODO: Cleanup this class to keep only the essential
	class Vectrix_API Application
	{
	public:
		Application();
		virtual ~Application();
		void run();

		void onEvent(Event& e);
		bool onWindowClose(WindowCloseEvent& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Window& window() { return *m_window; }
		static Device& device() { return VulkanContext::instance().getDevice(); }

		static Application& instance() { return *s_instance; }
		void renderImGui();
	private:
		std::unique_ptr<Window> m_window;
		std::unique_ptr<ShaderManager> m_shaderManager;
		ImGuiLayer* m_ImGuiLayer;
		bool m_running = true;

		LayerStack m_layerStack;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_instance;
	};

	// TODO: to be defined in the client
	Application* createApplication();
}