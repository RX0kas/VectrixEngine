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

		m_imGuiLayer = std::make_shared<ImGuiLayer>();
		m_imGuiLayer->OnAttach();
	}

	Application::~Application() {
		m_imGuiLayer.reset();
	}

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
		m_running = false;
		return true;
	}

	void Application::PushLayer(const Ref<Layer>& layer)
	{
		m_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(const Ref<Layer>& layer)
	{
		m_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::renderImGui() {
		for (const Ref<Layer>& layer : m_layerStack)
			layer->OnImGuiRender();
		m_imGuiLayer->OnImGuiRender();
	}
}
