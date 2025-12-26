#include "vcpch.h"

#include "Application.h"

#include <memory>

#include "GraphicAPI/Vulkan/VulkanContext.h"


namespace Vectrix {

#define BIND_EVENT_FN(x) [this](auto && PH1) { x(std::forward<decltype(PH1)>(PH1)); }

	Application* Application::_instance = nullptr;

	Application::Application() {
		VC_CORE_ASSERT(!_instance, "Application already exists!");
		_instance = this;

		_window = std::unique_ptr<Window>(Window::create());
		_window->setEventCallback(BIND_EVENT_FN(onEvent));
		_window->init();

		std::vector<Vertex> verticesVec{
	    	{ glm::vec3(0.5f, -0.5f, 0.0f), glm::vec4(1.0f) },
			{ glm::vec3(0.5f,  0.5f, 0.0f), glm::vec4(1.0f) },
			{ glm::vec3(0.0f, -0.5f, 0.0f), glm::vec4(1.0f) }
		};
		_vertexBuffer.reset(VertexBuffer::create(verticesVec, static_cast<uint32_t>(verticesVec.size())));

		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" }
		};

		_vertexBuffer->setLayout(layout);
		

		uint32_t indices[3] = { 0, 2, 1 };

		_indexBuffer.reset(IndexBuffer::create(indices, sizeof(indices) / sizeof(indices)));

		// TODO: Find a better way to store shaders
#ifdef VC_PLATFORM_WINDOWS
		_shader.reset(new Shader("E:\\v.vert.spv", "E:\\f.frag.spv", layout));
#elif defined(VC_PLATFORM_LINUX)
		_shader = std::make_unique<Shader>("./shaders/v.vert.spv", "./shaders/f.frag.spv", layout);
#endif

		_ImGuiLayer = new ImGuiLayer();
		PushOverlay(_ImGuiLayer);
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
			VulkanContext::instance().render();

			for (Layer* layer : _layerStack)
				layer->OnUpdate();

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