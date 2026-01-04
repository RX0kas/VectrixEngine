#include "vcpch.h"

#include "Application.h"

#include <memory>

#include "GraphicAPI/Vulkan/VulkanContext.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer.h"


namespace Vectrix {

#define BIND_EVENT_FN(x) [this](auto && PH1) { x(std::forward<decltype(PH1)>(PH1)); }

	Application* Application::_instance = nullptr;

	Application::Application() {
		VC_CORE_ASSERT(!_instance, "Application already exists!");
		_instance = this;

		_window = std::unique_ptr<Window>(Window::create());
		_window->setEventCallback(BIND_EVENT_FN(onEvent));
		_window->init();
		p_shaderManager = std::make_unique<ShaderManager>();

		p_ImGuiLayer = new ImGuiLayer();
		PushOverlay(p_ImGuiLayer);
	}

	Application::~Application() = default;

	void Application::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](auto && PH1) { return onWindowClose(std::forward<decltype(PH1)>(PH1)); });

		for (auto it = _layerStack.end(); it != _layerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::run() {

		while (_running) {
			for (Layer* layer : _layerStack) {
				layer->OnRender();
				layer->OnUpdate();
			}

			_window->onUpdate();
		}
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		_running = false;
		return true;
	}

	void Application::PushLayer(Layer* layer)
	{
		_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::renderImGui() {
		for (Layer* layer : _layerStack)
			layer->OnImGuiRender();
	}
}
