#ifndef VECTRIXWORKSPACE_EXAMPLELAYER_H
#define VECTRIXWORKSPACE_EXAMPLELAYER_H

#include "Vectrix.h"
#include "../CameraWidget.h"

class ExampleLayer : public Vectrix::Layer {
public:
	ExampleLayer() : Layer("Example") {
		m_cameraController.getCamera().setPosition({0.0f,0.0f,3.0f});
		m_cameraController.getCamera().setRotation({0.0f,-M_PI,0.0f});
		m_cameraWidget = std::make_shared<CameraWidget>(m_cameraController.getCamera());
		Vectrix::Application::instance().imguiLayer().addWidget(m_cameraWidget);

		m_model = Vectrix::MeshManager::loadModel("fox","./models/fox.obj");

		Vectrix::ShaderUniformLayout layout;
		layout.add("time",Vectrix::ShaderUniformType::Float);
		defaultShader = Vectrix::ShaderManager::createShader(p_defaultName, "./shaders/v.vert", "./shaders/f.frag",layout);
		customTexture = Vectrix::TextureManager::createTexture(p_defaultName, "./textures/fox.png");
	}

	void OnUpdate(const Vectrix::DeltaTime& dt) override {
		m_cameraController.onUpdate(dt);
	}

	void OnRender() override {
		Vectrix::Renderer::beginScene(m_cameraController.getCamera());
		defaultShader->setUniform("time",static_cast<float>(glfwGetTime()));
		defaultShader->setTexture(0,customTexture);
		Vectrix::Renderer::submit(*defaultShader.get(),*m_model);
		Vectrix::Renderer::endScene();
	}

	void OnEvent(Vectrix::Event &event) override {
		m_cameraController.onEvent(event);
	}

private:
	std::shared_ptr<CameraWidget> m_cameraWidget;
	Vectrix::PerspectiveCameraController m_cameraController;

	std::shared_ptr<Vectrix::Shader> defaultShader;
	std::shared_ptr<Vectrix::Texture> customTexture;
	std::shared_ptr<Vectrix::Model> m_model;
	std::shared_ptr<Vectrix::Framebuffer> m_framebuffer;
	const char* p_defaultName = "default";
};

#endif //VECTRIXWORKSPACE_EXAMPLELAYER_H