#include <Vectrix.h>

#include "Layers/ExampleLayer.h"

#include "Vectrix/EntryPoint.h"

class Sandbox : public Vectrix::Application {
public:
	Sandbox() {
		m_exampleLayer = std::make_shared<ExampleLayer>();
		PushLayer(m_exampleLayer);
	}
	~Sandbox() override = default;
private:
	std::shared_ptr<ExampleLayer> m_exampleLayer;
};

Vectrix::Application* Vectrix::createApplication() {
	return new Sandbox();
}

VC_SET_APP_INFO("Sandbox",0,1,0);