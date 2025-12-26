#include <Vectrix.h>

#include "imgui/imgui.h"

class ExampleLayer : public Vectrix::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		if (Vectrix::Input::isKeyPressed(VC_KEY_TAB))
			VC_TRACE("Tab key is pressed (poll)!");
	}

	virtual void OnImGuiRender() override
	{
		/*ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();*/
	}

	void OnEvent(Vectrix::Event& event) override
	{

	}

};

class Sandbox : public Vectrix::Application {
public:
	Sandbox() { 
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {}
};

Vectrix::Application* Vectrix::createApplication()
{
	return new Sandbox();
}