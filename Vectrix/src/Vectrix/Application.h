#pragma once

#include "Core.h"
#include "Window.h"
#include "GraphicAPI/Vulkan/rendering/Device.h"

#include "Vectrix/Layers/LayerStack.h"
#include "Events/Event.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Vectrix/Events/WindowEvent.h"

#include "ImGui/ImGuiLayer.h"
#include "Renderer/PerspectiveCamera.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexArray.h"

#include "Vectrix/Renderer/Shader.h"
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

		Window& window() { return *_window; }
		static Device& device() { return VulkanContext::instance().getDevice(); }

		static Application& instance() { return *_instance; }
		void renderImGui();
		[[nodiscard]] PerspectiveCamera& getCamera() const {return *m_Camera;}
	private:
		std::unique_ptr<Window> _window;
		std::unique_ptr<ShaderManager> p_shaderManager;
		ImGuiLayer* p_ImGuiLayer;
		bool _running = true;

		LayerStack _layerStack;

		std::unique_ptr<PerspectiveCamera> m_Camera;

		std::shared_ptr<VertexArray> m_vertexArray;

		std::unique_ptr<Shader> _shader;
		const char* p_defaultName = "default";
	private:
		static Application* _instance;
	};

	// TODO: to be defined in the client
	Application* createApplication();
}