#ifndef VECTRIXWORKSPACE_EXAMPLELAYER_H
#define VECTRIXWORKSPACE_EXAMPLELAYER_H

#include "Vectrix.h"
#include "imgui.h"
#include "Vectrix/Renderer/Models/ObjLoader.h"

class ExampleLayer : public Vectrix::Layer
{
public:
	ExampleLayer()
		: Layer("Example") {
		m_Camera = std::make_unique<Vectrix::PerspectiveCamera>();
		m_Camera->setPosition({0.0f,0.0f,0.0f});
		m_Camera->setRotation({0.0f,0.0f,0.0f});

		m_model = std::make_unique<Vectrix::Model>(Vectrix::Model::load("./models/suzanne.obj"));

		Vectrix::ShaderUniformLayout layout;
		layout.add("time", Vectrix::ShaderUniformType::Float);
		layout.finalize();

#ifdef VC_PLATFORM_WINDOWS
		ShaderManager::createShader(p_defaultName, "E:\\v.vert.spv", "E:\\f.frag.spv", layout);
#elif defined(VC_PLATFORM_LINUX)
		Vectrix::ShaderManager::createShader(p_defaultName, "./shaders/v.vert.spv", "./shaders/f.frag.spv",layout, Vectrix::getTinyObjLayout());
#endif
		def = Vectrix::ShaderManager::instance().get(p_defaultName);
	}

	void OnUpdate(Vectrix::DeltaTime ts) override
	{
		glm::vec3 m_CameraPosition = m_Camera->getPosition();
		glm::vec3 m_CameraRotation = m_Camera->getRotation();
		float m_CameraMoveSpeed = 1.0f;
		float m_CameraRotationSpeed = 1.0f;
		if (Vectrix::Input::isKeyPressed(VC_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if (Vectrix::Input::isKeyPressed(VC_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;

		if (Vectrix::Input::isKeyPressed(VC_KEY_W))
			m_CameraPosition.z += m_CameraMoveSpeed * ts;
		else if (Vectrix::Input::isKeyPressed(VC_KEY_S))
			m_CameraPosition.z -= m_CameraMoveSpeed * ts;
		if (Vectrix::Input::isKeyPressed(VC_KEY_Q))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Vectrix::Input::isKeyPressed(VC_KEY_E))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if (Vectrix::Input::isKeyPressed(VC_KEY_LEFT))
			m_CameraRotation.y -= m_CameraRotationSpeed * ts;
		else if (Vectrix::Input::isKeyPressed(VC_KEY_RIGHT))
			m_CameraRotation.y += m_CameraRotationSpeed * ts;

		if (Vectrix::Input::isKeyPressed(VC_KEY_UP))
			m_CameraRotation.x += m_CameraRotationSpeed * ts;
		else if (Vectrix::Input::isKeyPressed(VC_KEY_DOWN))
			m_CameraRotation.x -= m_CameraRotationSpeed * ts;

		m_Camera->setPosition(m_CameraPosition);
		m_Camera->setRotation(m_CameraRotation);
	}

	void OnRender() override {
		if (Vectrix::Renderer::BeginScene(*m_Camera)) {
			def->setUniform1f("time",glfwGetTime());
			Vectrix::Renderer::Submit(*def.get(),*m_model);

			Vectrix::Renderer::EndScene();
		}
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Debug Camera");
		float pos[3] = {m_Camera->getPosition().x,m_Camera->getPosition().y,m_Camera->getPosition().z};
		if (ImGui::SliderFloat3("Position",pos,-M_PI*2,M_PI*2)) {
			m_Camera->setPosition({pos[0],pos[1],pos[2]});
		}
		float rot[3] = {m_Camera->getRotation().x,m_Camera->getRotation().y,m_Camera->getRotation().z};
		if (ImGui::SliderFloat3("Rotation",rot,-M_PI*2,M_PI*2)) {
			m_Camera->setRotation({rot[0],rot[1],rot[2]});
		}
		ImGui::End();
	}

	void OnEvent(Vectrix::Event &event) override {
		if (event.getEventType()==Vectrix::EventType::WindowResize)
			m_Camera->recalculateMatrices();
	}

private:
	Vectrix::Scope<Vectrix::PerspectiveCamera> m_Camera;

	Vectrix::Ref<Vectrix::VertexArray> m_vertexArray;

	Vectrix::Scope<Vectrix::Shader> _shader;
	Vectrix::Ref<Vectrix::Shader> def;
	Vectrix::Scope<Vectrix::Model> m_model;
	const char* p_defaultName = "default";
};

#endif //VECTRIXWORKSPACE_EXAMPLELAYER_H