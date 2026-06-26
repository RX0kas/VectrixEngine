#ifndef VECTRIXWORKSPACE_EXAMPLELAYER_H
#define VECTRIXWORKSPACE_EXAMPLELAYER_H

#include "Vectrix.h"
#include "../CameraWidget.h"
#include "Vectrix/Assets/AssetsManager.h"
#include "Vectrix/Rendering/GraphicsContext.h"
#include "Vectrix/Scene/Components/CameraComponent.h"

class ExampleLayer : public Vectrix::Layer {
public:
	ExampleLayer() : Layer("Example"),m_activeScene("ExampleScene") {}

	void OnAttach() override {
		m_cameraEntity = m_activeScene.createEntity("Camera");
		m_cameraEntity->addComponent<Vectrix::CameraComponent>();
		m_cameraWidget = std::make_shared<CameraWidget>(m_cameraEntity);
		Vectrix::Application::instance().imguiLayer().addWidget(m_cameraWidget);

		auto s = Vectrix::AssetsManager::load<Vectrix::Shader>("./shaders/sandbox.vcshader");
		if (s.first!=Vectrix::SUCCESS) {
			VC_ERROR("Can't load shader sandbox");
		}
		m_shader = s.second;
		auto t = Vectrix::AssetsManager::load<Vectrix::Texture>("./textures/fox.png");
		if (t.first!=Vectrix::SUCCESS) {
			VC_ERROR("Can't load texture for fox");
		}
		m_foxTexture = t.second;

		auto m = Vectrix::AssetsManager::load<Vectrix::Mesh>("./models/fox.obj");
		if (m.first!=Vectrix::SUCCESS) {
			VC_ERROR("Can't load model for fox");
		}
		m_meshFox = m.second;

		m_fox = m_activeScene.createEntity("Fox");
		m_fox->addComponent<Vectrix::MeshRendererComponent>(m_meshFox,m_shader,m_foxTexture);
	}

	void OnUpdate(const Vectrix::DeltaTime& dt) override {
		m_activeScene.OnUpdate(dt);
	}

	void OnRender() override {
		Vectrix::Renderer::beginScene(m_cameraEntity->getComponent<Vectrix::CameraComponent>().camera);
		m_activeScene.OnRender();
		Vectrix::Renderer::endScene();
	}

	void OnEvent(Vectrix::Event &event) override {
		m_cameraEntity->getComponent<Vectrix::CameraComponent>().camera.recalculateMatrices();
	}

private:
	std::shared_ptr<CameraWidget> m_cameraWidget;
	std::shared_ptr<Vectrix::Entity> m_cameraEntity;

	std::shared_ptr<Vectrix::Shader> m_shader;
	std::shared_ptr<Vectrix::Texture> m_foxTexture;
	std::shared_ptr<Vectrix::Mesh> m_meshFox;
	std::shared_ptr<Vectrix::Framebuffer> m_framebuffer;
	std::shared_ptr<Vectrix::Entity> m_fox;
	Vectrix::Scene m_activeScene;
	const char* p_defaultName = "default";
};

#endif //VECTRIXWORKSPACE_EXAMPLELAYER_H