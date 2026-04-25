#ifndef VECTRIXWORKSPACE_EXAMPLELAYER_H
#define VECTRIXWORKSPACE_EXAMPLELAYER_H

#include "Vectrix.h"
#include "../CameraWidget.h"

class ExampleLayer : public Vectrix::Layer {
public:
	ExampleLayer() : Layer("Example"),m_activeScene() {
		m_cameraController.getCamera().setPosition({0.0f,0.0f,3.0f});
		m_cameraController.getCamera().setRotation({0.0f,-M_PI,0.0f});
		m_cameraWidget = std::make_shared<CameraWidget>(m_cameraController.getCamera());
		Vectrix::Application::instance().imguiLayer().addWidget(m_cameraWidget);

		Vectrix::ShaderUniformLayout layout;
		layout.add("time",Vectrix::ShaderUniformType::Float);
		m_shader = Vectrix::ShaderManager::createShader(p_defaultName, "./shaders/v.vert", "./shaders/f.frag",layout);
		m_foxTexture = Vectrix::TextureManager::createTexture(p_defaultName, "./textures/fox.png");

		m_fox = m_activeScene.createEntity("Fox");
		m_fox.addComponent<Vectrix::TransformComponent>();
		m_fox.addComponent<Vectrix::MeshComponent>("./models/fox.obj",*m_shader,m_foxTexture);
	}

	void OnUpdate(const Vectrix::DeltaTime& dt) override {
		m_cameraController.onUpdate(dt);
		m_activeScene.OnUpdate(dt);
	}

	void OnRender() override {
		Vectrix::Renderer::beginScene(m_cameraController.getCamera());
		m_activeScene.OnRender();
		Vectrix::Renderer::endScene();
	}

	void OnEvent(Vectrix::Event &event) override {
		m_cameraController.onEvent(event);
	}

private:
	std::shared_ptr<CameraWidget> m_cameraWidget;
	Vectrix::PerspectiveCameraController m_cameraController;

	std::shared_ptr<Vectrix::Shader> m_shader;
	std::shared_ptr<Vectrix::Texture> m_foxTexture;
	std::shared_ptr<Vectrix::Framebuffer> m_framebuffer;
	Vectrix::Entity m_fox = Vectrix::Entity::nullEntity();
	Vectrix::Scene m_activeScene;
	const char* p_defaultName = "default";
};

#endif //VECTRIXWORKSPACE_EXAMPLELAYER_H