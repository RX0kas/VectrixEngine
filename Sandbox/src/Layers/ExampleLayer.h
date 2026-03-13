#ifndef VECTRIXWORKSPACE_EXAMPLELAYER_H
#define VECTRIXWORKSPACE_EXAMPLELAYER_H

#include "Vectrix.h"
#include "imgui.h"
#include "../CameraWidget.h"

class ExampleLayer : public Vectrix::Layer
{
public:
	ExampleLayer() : Layer("Example") {
		m_cameraController.getCamera().setPosition({0.0f,0.0f,3.0f});
		m_cameraController.getCamera().setRotation({0.0f,-M_PI,0.0f});
		m_cameraWidget = std::make_shared<CameraWidget>(m_cameraController.getCamera());
		Vectrix::Application::instance().imguiLayer().addWidget(m_cameraWidget);

		m_model = std::make_unique<Vectrix::Model>(Vectrix::Model::load("./models/fox.obj"));

		Vectrix::ShaderUniformLayout layout;
		layout.add("time",Vectrix::ShaderUniformType::Float);
#ifdef VC_PLATFORM_WINDOWS
		Vectrix::ShaderManager::createShader(p_defaultName, ".\\shaders\\v.vert", ".\\shaders\\f.frag",layout, Vectrix::getTinyObjLayout(),true);
		Vectrix::TextureManager::createTexture(p_defaultName, ".\\textures\\fox.png");
		Vectrix::TextureManager::createTexture("t2", "./textures/test.png");
#elif defined(VC_PLATFORM_LINUX)
		Vectrix::ShaderManager::createShader(p_defaultName, "./shaders/v.vert", "./shaders/f.frag",layout, Vectrix::getTinyObjLayout(),true);
		Vectrix::TextureManager::createTexture(p_defaultName, "./textures/fox.png");
#endif

		defaultShader = Vectrix::ShaderManager::instance().get(p_defaultName);
		customTexture = Vectrix::TextureManager::instance().get(p_defaultName);

		VC_CORE_INFO("Is compatible: {}",Vectrix::Profiler::isCompatible("Profile-Shutdown.json"));
		VC_CORE_INFO("Is isValidFormat: {}",Vectrix::Profiler::isValidFormat("Profile-Shutdown.json"));
	}

	void OnUpdate(const Vectrix::DeltaTime& dt) override
	{
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
	Vectrix::Ref<CameraWidget> m_cameraWidget;
	Vectrix::PerspectiveCameraController m_cameraController;

	Vectrix::Ref<Vectrix::Shader> defaultShader;
	Vectrix::Ref<Vectrix::Texture> customTexture;
	Vectrix::Own<Vectrix::Model> m_model;
	const char* p_defaultName = "default";
};

#endif //VECTRIXWORKSPACE_EXAMPLELAYER_H