#include "vcpch.h"

#include "Application.h"

#include <memory>

#include "Core/DeltaTime.h"
#include "Debug/Profiler.h"
#include "Rendering/RenderCommand.h"
#include "Rendering/Renderer.h"


namespace Vectrix {

#define BIND_EVENT_FN(x) [this](auto && PH1) { x(std::forward<decltype(PH1)>(PH1)); }

	Application* Application::s_instance = nullptr;

	Application::Application() {
		VC_CORE_ASSERT(!s_instance, "Application already exists!");
		s_instance = this;

		VC_PROFILER_FUNCTION();

		m_window = Ref<Window>(Window::create());
		m_window->setEventCallback(BIND_EVENT_FN(onEvent));
		m_window->init();
		auto tm = new TextureManager();
		m_textureManager = std::unique_ptr<TextureManager>(tm);
		auto sm = new ShaderManager();
		m_shaderManager = std::unique_ptr<ShaderManager>(sm);
		auto i = new ImGuiLayer();
		m_imGuiLayer = std::shared_ptr<ImGuiLayer>(i);
		m_imGuiLayer->OnAttach();
	}

	Application::~Application() {
		VC_PROFILER_FUNCTION();
		m_imGuiLayer.reset();
	}

	void Application::onEvent(Event& e) {
		VC_PROFILER_FUNCTION();
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(VC_BIND_EVENT_FN_RETURN(onWindowClose));

		for (auto it = m_layerStack.end(); it != m_layerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::run() {
		VC_PROFILER_FUNCTION();
		m_window->show();
		while (m_running) {
			auto time = static_cast<float>(glfwGetTime());
			m_deltaTime = time - m_LastFrameTime;
			m_LastFrameTime = time;
			if (RenderCommand::setupFrame()) {
				for (const Ref<Layer>& layer : m_layerStack) {
					layer->OnRender();
					layer->OnUpdate(m_deltaTime);
				}
				RenderCommand::endFrame();
				m_imGuiLayer->OnRender();
				m_imGuiLayer->OnUpdate(m_deltaTime);
				RenderCommand::sendFrame();
			}

			m_window->onUpdate();
		}
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		VC_PROFILER_FUNCTION();
		m_running = false;
		return true;
	}

	void Application::PushLayer(const Ref<Layer>& layer) {
		VC_PROFILER_FUNCTION();
		m_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(const Ref<Layer>& layer) {
		VC_PROFILER_FUNCTION();
		m_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::renderImGui() {
		VC_PROFILER_FUNCTION();
		for (const Ref<Layer>& layer : m_layerStack)
			layer->OnImGuiRender();
		m_imGuiLayer->OnImGuiRender();
	}
}
