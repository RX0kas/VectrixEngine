#ifndef VECTRIXWORKSPACE_EDITORLAYER_H
#define VECTRIXWORKSPACE_EDITORLAYER_H
#include "imgui.h"
#include "ImGuizmo.h"
#include "Vectrix.h"
#include "Panels/SceneHierarchyPanel.h"

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
    	Entity pickEntity(glm::vec2 mousePos);
    	glm::vec3 screenToWorldRay(glm::vec2 mousePos);
		std::shared_ptr<Shader> m_viewportShader;
		std::shared_ptr<Framebuffer> m_framebuffer;

    	std::shared_ptr<Texture> m_foxTexture;
    	std::shared_ptr<Mesh> m_foxMesh;
    	glm::vec2 m_viewportSize;
    	glm::vec2 m_viewportPos{1};
    	bool m_viewportFocused = false, m_viewportHovered = false;
    	bool m_mustResize = false;

    	std::shared_ptr<Scene> m_activeScene;
    	Entity m_foxEntity;
    	Entity m_cameraEntity;
    	Camera* m_camera;
    	float m_cameraRotationSpeed = 50.0f;
    	float m_cameraMoveSpeed = 1.5f;

    	int m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;

    	// Panels
    	SceneHierarchyPanel m_sceneHierarchyPanel;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_EDITORLAYER_H