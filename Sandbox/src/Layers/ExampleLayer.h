#ifndef VECTRIXWORKSPACE_EXAMPLELAYER_H
#define VECTRIXWORKSPACE_EXAMPLELAYER_H

#include "Vectrix.h"
#include "../CameraWidget.h"
#include "Vectrix/Scene/Components/CameraComponent.h"

class ExampleLayer : public Vectrix::Layer {
public:
	ExampleLayer() : Layer("Example") {
	}

	void OnAttach() override {
		m_cameraEntity = m_activeScene.createEntity("Camera");
		m_cameraEntity.getComponent<Vectrix::TransformComponent>().position = {0.0f,0.0f,3.0f};
		m_cameraEntity.getComponent<Vectrix::TransformComponent>().rotation = {0.0f,-M_PI,0.0f};
		m_cameraEntity.addComponent<Vectrix::CameraComponent>(m_cameraEntity.getComponent<Vectrix::TransformComponent>());
		m_cameraWidget = std::make_shared<CameraWidget>(m_cameraEntity);
		Vectrix::Application::instance().imguiLayer().addWidget(m_cameraWidget);

		Vectrix::ShaderUniformLayout layout;
		layout.add("time",Vectrix::ShaderUniformType::Float);
		m_shader = Vectrix::ShaderManager::createShader(p_defaultName, "./shaders/v.vert", "./shaders/f.frag",layout);
		m_foxTexture = Vectrix::TextureManager::createTexture(p_defaultName, "./textures/fox.png");

		m_fox = m_activeScene.createEntity("Fox");
		m_fox.addComponent<Vectrix::MeshRendererComponent>("./models/fox.obj",m_shader,m_foxTexture);
	}

	void OnUpdate(const Vectrix::DeltaTime& dt) override {
		m_activeScene.OnUpdate(dt);
	}

	void OnRender() override {
		Vectrix::Renderer::beginScene(m_cameraEntity.getComponent<Vectrix::CameraComponent>().camera);
		m_activeScene.OnRender();
		Vectrix::Renderer::endScene();
	}

	void OnEvent(Vectrix::Event &event) override {
		m_cameraEntity.getComponent<Vectrix::CameraComponent>().camera.recalculateMatrices();
	}

private:
	std::shared_ptr<CameraWidget> m_cameraWidget;
	Vectrix::Entity m_cameraEntity;

	std::shared_ptr<Vectrix::Shader> m_shader;
	std::shared_ptr<Vectrix::Texture> m_foxTexture;
	std::shared_ptr<Vectrix::Framebuffer> m_framebuffer;
	Vectrix::Entity m_fox = Vectrix::Entity::nullEntity();
	Vectrix::Scene m_activeScene;
	const char* p_defaultName = "default";
};

#endif //VECTRIXWORKSPACE_EXAMPLELAYER_H