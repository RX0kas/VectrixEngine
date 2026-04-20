#ifndef VECTRIXWORKSPACE_EDITORLAYER_H
#define VECTRIXWORKSPACE_EDITORLAYER_H
#include "Vectrix.h"

namespace Vectrix {
    class EditorLayer : public Layer {
    public:
		EditorLayer();

		void OnUpdate(const DeltaTime& dt) override;

		void OnRender() override;

		void OnRenderOffscreen() override;

		void OnEvent(Event &event) override;

		void OnImGuiRender() override;
	private:
		PerspectiveCameraController m_cameraController;

		std::shared_ptr<Shader> m_viewportShader;
		std::shared_ptr<Framebuffer> m_framebuffer;

    	std::shared_ptr<Texture> m_testTexture;
    	std::shared_ptr<Model> m_testModel;

    	glm::vec2 m_viewportSize;
    	bool m_mustResize = false;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_EDITORLAYER_H