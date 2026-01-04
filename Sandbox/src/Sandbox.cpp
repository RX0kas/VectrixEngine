#include <Vectrix.h>

#include "imgui.h"

class ExampleLayer : public Vectrix::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		
	}

	void OnImGuiRender() override
	{
		Vectrix::PerspectiveCamera& m_Camera = Vectrix::Application::instance().getCamera();
		ImGui::Begin("Debug Camera");
		float pos[3] = {m_Camera.getPosition().x,m_Camera.getPosition().y,m_Camera.getPosition().z};
		if (ImGui::SliderFloat3("Position",pos,-10,10)) {
			m_Camera.setPosition({pos[0],pos[1],pos[2]});
		}
		float rot[3] = {m_Camera.getRotation().x,m_Camera.getRotation().y,m_Camera.getRotation().z};
		if (ImGui::SliderFloat3("Rotation",rot,-10,10)) {
			m_Camera.setRotation({rot[0],rot[1],rot[2]});
		}
		ImGui::End();
	}
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