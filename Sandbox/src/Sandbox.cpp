#include <Vectrix.h>

#include "Layers/ExampleLayer.h"

class Sandbox : public Vectrix::Application {
public:
	Sandbox() {
		m_exampleLayer = std::make_shared<ExampleLayer>();
		PushLayer(m_exampleLayer);
	}
	~Sandbox() override = default;
private:
	Vectrix::Ref<ExampleLayer> m_exampleLayer;
};

Vectrix::Application* Vectrix::createApplication()
{
	return new Sandbox();
}