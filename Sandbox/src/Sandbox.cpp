#include <Vectrix.h>

#include "Layers/ExampleLayer.h"

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