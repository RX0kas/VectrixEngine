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

    	void OnAttach() override;
	private:
		std::shared_ptr<Shader> m_viewportShader;
		std::shared_ptr<Framebuffer> m_framebuffer;

    	std::shared_ptr<Texture> m_testTexture;
    	glm::vec2 m_viewportSize;
    	bool m_viewportFocused = false, m_viewportHovered = false;
    	bool m_mustResize = false;

    	std::shared_ptr<Scene> m_activeScene;
    	Entity m_foxEntity;
    	Entity m_cameraEntity;
    	Camera* m_camera;
    	float m_cameraRotationSpeed = 5.0f;
    	float m_cameraMoveSpeed = 1.5f;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_EDITORLAYER_H