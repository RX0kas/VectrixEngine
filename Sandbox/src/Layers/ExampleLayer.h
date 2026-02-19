#ifndef VECTRIXWORKSPACE_EXAMPLELAYER_H
#define VECTRIXWORKSPACE_EXAMPLELAYER_H

#include "Vectrix.h"
#include "imgui.h"
#include "../CameraWidget.h"
#include "Vectrix/Utils/Math.h"

class ExampleLayer : public Vectrix::Layer
{
public:
	ExampleLayer() : Layer("Example") {
		m_camera = std::make_unique<Vectrix::PerspectiveCamera>();
		m_camera->setPosition({0.0f,0.0f,3.0f});
		m_camera->setRotation({0.0f,-M_PI,0.0f});

		m_cameraWidget = new CameraWidget(*m_camera);

		m_model = std::make_unique<Vectrix::Model>(Vectrix::Model::load("./models/cat.obj"));

		Vectrix::ShaderUniformLayout layout;
		layout.add("time",Vectrix::ShaderUniformType::Float);
#ifdef VC_PLATFORM_WINDOWS
		Vectrix::ShaderManager::createShader(p_defaultName, ".\\shaders\\v.vert", ".\\shaders\\f.frag",layout, Vectrix::getTinyObjLayout(),true);
#elif defined(VC_PLATFORM_LINUX)
		Vectrix::ShaderManager::createShader(p_defaultName, "./shaders/v.vert", "./shaders/f.frag",layout, Vectrix::getTinyObjLayout(),true);
#endif
		def = Vectrix::ShaderManager::instance().get(p_defaultName);
	}

	void OnUpdate(Vectrix::DeltaTime ts) override
	{
		glm::vec3 m_CameraPosition = m_camera->getPosition();
		glm::vec3 m_CameraRotation = m_camera->getRotation();
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

		m_camera->setPosition(m_CameraPosition);
		m_camera->setRotation(m_CameraRotation);
	}

	void OnRender() override {
		Vectrix::Renderer::BeginScene(*m_camera);
		def->setUniform("time",static_cast<float>(glfwGetTime()));

		Vectrix::Renderer::Submit(*def.get(),*m_model);
		Vectrix::Renderer::EndScene();
	}

	void OnEvent(Vectrix::Event &event) override {
		if (event.getEventType()==Vectrix::EventType::WindowResize)
			m_camera->recalculateMatrices();
	}

private:
	Vectrix::Own<Vectrix::PerspectiveCamera> m_camera;
	CameraWidget *m_cameraWidget;

	Vectrix::Ref<Vectrix::Shader> def;
	Vectrix::Own<Vectrix::Model> m_model;
	const char* p_defaultName = "default";
};

#endif //VECTRIXWORKSPACE_EXAMPLELAYER_H