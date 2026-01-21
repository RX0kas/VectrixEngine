#include "vcpch.h"

#include "Application.h"

#include <memory>

#include "Core/DeltaTime.h"
#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer.h"


namespace Vectrix {

#define BIND_EVENT_FN(x) [this](auto && PH1) { x(std::forward<decltype(PH1)>(PH1)); }

	Application* Application::s_instance = nullptr;

	Application::Application() {
		VC_CORE_ASSERT(!s_instance, "Application already exists!");
		s_instance = this;

		m_window = Ref<Window>(Window::create());
		m_window->setEventCallback(BIND_EVENT_FN(onEvent));
		m_window->init();
		m_shaderManager = std::make_unique<ShaderManager>();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application() = default;

	void Application::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](auto && PH1) { return onWindowClose(std::forward<decltype(PH1)>(PH1)); });

		for (auto it = m_layerStack.end(); it != m_layerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::run() {

		while (m_running) {
			auto time = static_cast<float>(glfwGetTime());
			DeltaTime deltaTime = time - m_LastFrameTime;
			m_LastFrameTime = time;
			for (Layer* layer : m_layerStack) {
				layer->OnRender();
				layer->OnUpdate(deltaTime);
			}

			m_window->onUpdate();
		}
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		m_running = false;
		return true;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::renderImGui() {
		for (Layer* layer : m_layerStack)
			layer->OnImGuiRender();
	}
}
