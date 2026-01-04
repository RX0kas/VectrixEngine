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

		m_Camera = std::make_unique<PerspectiveCamera>();
		m_Camera->setPosition({0.0f,0.0f,-2.5f});

		
		std::vector<Vertex> verticesVec{
		    // LEFT (-X)
		    { {-0.5f, -0.5f, -0.5f}, {0.9f, 0.9f, 0.9f, 1.0f} },
		    { {-0.5f, -0.5f,  0.5f}, {0.9f, 0.9f, 0.9f, 1.0f} },
		    { {-0.5f,  0.5f,  0.5f}, {0.9f, 0.9f, 0.9f, 1.0f} },

		    { {-0.5f, -0.5f, -0.5f}, {0.9f, 0.9f, 0.9f, 1.0f} },
		    { {-0.5f,  0.5f,  0.5f}, {0.9f, 0.9f, 0.9f, 1.0f} },
		    { {-0.5f,  0.5f, -0.5f}, {0.9f, 0.9f, 0.9f, 1.0f} },

		    // RIGHT (+X)
		    { { 0.5f, -0.5f, -0.5f}, {0.8f, 0.8f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f,  0.5f}, {0.8f, 0.8f, 0.1f, 1.0f} },
		    { { 0.5f, -0.5f,  0.5f}, {0.8f, 0.8f, 0.1f, 1.0f} },

		    { { 0.5f, -0.5f, -0.5f}, {0.8f, 0.8f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f, -0.5f}, {0.8f, 0.8f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f,  0.5f}, {0.8f, 0.8f, 0.1f, 1.0f} },

		    // TOP (-Y) (Vulkan Y vers le bas)
		    { {-0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f, 1.0f} },
		    { {-0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.1f, 1.0f} },
		    { { 0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.1f, 1.0f} },

		    { {-0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f, 1.0f} },
		    { { 0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.1f, 1.0f} },
		    { { 0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f, 1.0f} },

		    // BOTTOM (+Y)
		    { {-0.5f,  0.5f, -0.5f}, {0.8f, 0.1f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f,  0.5f}, {0.8f, 0.1f, 0.1f, 1.0f} },
		    { {-0.5f,  0.5f,  0.5f}, {0.8f, 0.1f, 0.1f, 1.0f} },

		    { {-0.5f,  0.5f, -0.5f}, {0.8f, 0.1f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f, -0.5f}, {0.8f, 0.1f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f,  0.5f}, {0.8f, 0.1f, 0.1f, 1.0f} },

		    // FRONT (+Z)
		    { {-0.5f, -0.5f,  0.5f}, {0.1f, 0.1f, 0.8f, 1.0f} },
		    { {-0.5f,  0.5f,  0.5f}, {0.1f, 0.1f, 0.8f, 1.0f} },
		    { { 0.5f,  0.5f,  0.5f}, {0.1f, 0.1f, 0.8f, 1.0f} },

		    { {-0.5f, -0.5f,  0.5f}, {0.1f, 0.1f, 0.8f, 1.0f} },
		    { { 0.5f,  0.5f,  0.5f}, {0.1f, 0.1f, 0.8f, 1.0f} },
		    { { 0.5f, -0.5f,  0.5f}, {0.1f, 0.1f, 0.8f, 1.0f} },

		    // BACK (-Z)
		    { {-0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f} },
		    { {-0.5f,  0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f} },

		    { {-0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f} },
		    { { 0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f} },
		    { { 0.5f,  0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f} },
		};



		std::shared_ptr<VertexBuffer> vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::create(verticesVec, static_cast<uint32_t>(verticesVec.size())));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->setLayout(layout);
		
		m_vertexArray = std::shared_ptr<VertexArray>(VertexArray::create());
		m_vertexArray->addVertexBuffer(vertexBuffer);

		//uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		//_indexBuffer.reset(IndexBuffer::create(indices, sizeof(indices)/sizeof(uint32_t)));


#ifdef VC_PLATFORM_WINDOWS
		ShaderManager::createShader(p_defaultName,"E:\\v.vert.spv", "E:\\f.frag.spv",layout);
#elif defined(VC_PLATFORM_LINUX)
		ShaderManager::createShader(p_defaultName,"./shaders/v.vert.spv", "./shaders/f.frag.spv",layout);
#endif

		p_ImGuiLayer = new ImGuiLayer();
		PushOverlay(p_ImGuiLayer);
	}

	Application::~Application() = default;

	void Application::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](auto && PH1) { return onWindowClose(std::forward<decltype(PH1)>(PH1)); });

		if (e.getEventType()==EventType::WindowResize)
			m_Camera->recalculateMatrices();


		for (auto it = _layerStack.end(); it != _layerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::run() {
		std::shared_ptr<Shader> def = ShaderManager::instance().get(p_defaultName);
		while (_running) {
			Renderer::BeginScene(*m_Camera);

			Renderer::Submit(def,m_vertexArray);
			RenderCommand::drawIndexed(m_vertexArray);

			Renderer::EndScene();

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
