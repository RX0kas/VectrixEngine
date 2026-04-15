#include "vcpch.h"

#include "Application.h"

#include <memory>

#include "Core/DeltaTime.h"
#include "Debug/Profiler.h"
#include "Rendering/GraphicsContext.h"
#include "Rendering/RenderCommand.h"
#include "Rendering/Mesh/MeshManager.h"
#include "Rendering/Shaders/ShaderManager.h"
#include "Rendering/Textures/TextureManager.h"


namespace Vectrix {

#define BIND_EVENT_FN(x) [this](auto && PH1) { x(std::forward<decltype(PH1)>(PH1)); }

	Application* Application::s_instance = nullptr;

	Application::Application() {
		VC_PROFILER_FUNCTION();
		VC_CORE_ASSERT(!s_instance, "Application already exists!");
		s_instance = this;

		m_window = std::unique_ptr<Window>(Window::create());
		m_window->setEventCallback(BIND_EVENT_FN(onEvent));
		m_window->init();
		auto tm = new TextureManager();
		m_textureManager = std::unique_ptr<TextureManager>(tm);
		auto sm = new ShaderManager();
		m_shaderManager = std::unique_ptr<ShaderManager>(sm);
		auto mm = new MeshManager();
		m_meshManager = std::unique_ptr<MeshManager>(mm);
		auto i = new ImGuiLayer();
		m_imGuiLayer = std::unique_ptr<ImGuiLayer>(i);
		m_imGuiLayer->OnAttach();
	}

	Application::~Application() {
		VC_PROFILER_FUNCTION();
		m_layerStack.destroy();
		m_imGuiLayer.reset();
		m_shaderManager.reset();
		m_textureManager.reset();
		m_meshManager.reset();
		m_window.reset();
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
		GraphicsContext::uploadAllMeshData();
		VC_PROFILER_FUNCTION();
		m_window->show();
		while (m_running) {
			const auto time = static_cast<float>(glfwGetTime());
			m_deltaTime = time - m_LastFrameTime;
			m_LastFrameTime = time;
			if (RenderCommand::canRender()) {
				for (auto& layer : m_layerStack) {
					layer->OnRenderOffscreen();
					layer->OnUpdate(m_deltaTime);
				}

				RenderCommand::beginFrame();
				for (const std::shared_ptr<Layer>& layer : m_layerStack) {
					layer->OnRender();
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

	void Application::PushLayer(const std::shared_ptr<Layer>& layer) {
		VC_PROFILER_FUNCTION();
		m_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(const std::shared_ptr<Layer>& layer) {
		VC_PROFILER_FUNCTION();
		m_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::renderImGui() {
		VC_PROFILER_FUNCTION();
		for (const std::shared_ptr<Layer>& layer : m_layerStack)
			layer->OnImGuiRender();
		m_imGuiLayer->OnImGuiRender();
	}
}
