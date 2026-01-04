#include <Vectrix.h>

#include "imgui.h"

class ExampleLayer : public Vectrix::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		m_Camera = std::make_unique<Vectrix::PerspectiveCamera>();
		m_Camera->setPosition({0.0f,0.0f,-2.5f});


		std::vector<Vectrix::Vertex> verticesVec{
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



		std::shared_ptr<Vectrix::VertexBuffer> vertexBuffer = std::shared_ptr<Vectrix::VertexBuffer>(Vectrix::VertexBuffer::create(verticesVec, static_cast<uint32_t>(verticesVec.size())));
		Vectrix::BufferLayout layout = {
			{Vectrix::ShaderDataType::Float3, "a_Position" },
			{ Vectrix::ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->setLayout(layout);

		m_vertexArray = std::shared_ptr<Vectrix::VertexArray>(Vectrix::VertexArray::create());
		m_vertexArray->addVertexBuffer(vertexBuffer);

		//uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		//_indexBuffer.reset(IndexBuffer::create(indices, sizeof(indices)/sizeof(uint32_t)));


#ifdef VC_PLATFORM_WINDOWS
		ShaderManager::createShader(p_defaultName,"E:\\v.vert.spv", "E:\\f.frag.spv",layout);
#elif defined(VC_PLATFORM_LINUX)
		Vectrix::ShaderManager::createShader(p_defaultName,"./shaders/v.vert.spv", "./shaders/f.frag.spv",layout);
#endif
		 def = Vectrix::ShaderManager::instance().get(p_defaultName);
	}

	void OnUpdate() override
	{

	}

	void OnRender() override {
		Vectrix::Renderer::BeginScene(*m_Camera);

		Vectrix::Renderer::Submit(def,m_vertexArray);
		Vectrix::RenderCommand::drawIndexed(m_vertexArray);

		Vectrix::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Debug Camera");
		float pos[3] = {m_Camera->getPosition().x,m_Camera->getPosition().y,m_Camera->getPosition().z};
		if (ImGui::SliderFloat3("Position",pos,-10,10)) {
			m_Camera->setPosition({pos[0],pos[1],pos[2]});
		}
		float rot[3] = {m_Camera->getRotation().x,m_Camera->getRotation().y,m_Camera->getRotation().z};
		if (ImGui::SliderFloat3("Rotation",rot,-10,10)) {
			m_Camera->setRotation({rot[0],rot[1],rot[2]});
		}
		ImGui::End();
	}

	void OnEvent(Vectrix::Event &event) override {
		if (event.getEventType()==Vectrix::EventType::WindowResize)
			m_Camera->recalculateMatrices();
	}

private:
	std::unique_ptr<Vectrix::PerspectiveCamera> m_Camera;

	std::shared_ptr<Vectrix::VertexArray> m_vertexArray;

	std::unique_ptr<Vectrix::Shader> _shader;
	std::shared_ptr<Vectrix::Shader> def;
	
	const char* p_defaultName = "default";
};

class Sandbox : public Vectrix::Application {
public:
	Sandbox() { 
		PushLayer(new ExampleLayer());
	}
	~Sandbox() override = default;
};

Vectrix::Application* Vectrix::createApplication()
{
	return new Sandbox();
}