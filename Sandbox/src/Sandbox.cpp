#include <Vectrix.h>

#include "Layers/ExampleLayer.h"

#include "Vectrix/EntryPoint.h"

class Sandbox : public Vectrix::Application {
public:
	Sandbox() {
		PushLayer<ExampleLayer>();
	}
	~Sandbox() override = default;
};

Vectrix::Application* Vectrix::createApplication() {
	return new Sandbox();
}

VC_SET_APP_INFO("Sandbox",0,1,0);