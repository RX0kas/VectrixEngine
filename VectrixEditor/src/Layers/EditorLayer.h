#ifndef VECTRIXWORKSPACE_EDITORLAYER_H
#define VECTRIXWORKSPACE_EDITORLAYER_H
#include "Vectrix/Rendering/Camera/EditorCamera.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "StartupLayer.h"
#include "Vectrix.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/SettingsPanel.h"
#include "Undo/UndoHistory.h"

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
    	void OnAttach(const JsonObject& data) override;
    	void openScene(const std::filesystem::path& path);
    	std::shared_ptr<Scene> getActiveScene() { return m_activeScene; }
	private:
    	void showSaveDialog();
    	void showOpenDialog();
    	void showOpenProjectDialog();
    	void processPendingSceneLoad();
    	void processPendingProjectLoad();
    	void applyLiveSettings(); // re-reads the settings that take effect without a relaunch
    	std::shared_ptr<Entity> pickEntity(glm::vec2 mousePos);
    	glm::vec3 screenToWorldRay(glm::vec2 mousePos);
		std::shared_ptr<Framebuffer> m_framebuffer;
    	std::unique_ptr<EditorCamera> m_camera;

    	glm::vec2 m_viewportSize;
    	glm::vec2 m_viewportPos{1};
    	bool m_viewportFocused = false, m_viewportHovered = false;
    	bool m_mustResize = false;

    	std::shared_ptr<Scene> m_activeScene;
    	std::string m_pendingScenePath{};
    	std::string m_pendingProjectPath{};
    	std::filesystem::path m_projectDirectory; ///< Root of the open project, where its settings.vectrix.json lives

    	float m_cameraRotationSpeed = 50.0f;
    	float m_cameraMoveSpeed = 1.5f;
    	float m_gizmoTranslationSnap = 0.5f;
    	float m_gizmoRotationSnap = 45.0f;
    	glm::vec4 m_appliedClearColor{0.0f, 0.0f, 0.0f, 1.0f};
    	std::string m_appliedTheme = "dark";

    	int m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;

    	// Panels
    	std::unique_ptr<SceneHierarchyPanel> m_sceneHierarchyPanel;
    	std::unique_ptr<ContentBrowserPanel> m_contentBrowserPanel;
    	std::unique_ptr<SettingsPanel> m_settingPanel;
    	bool m_graphicDebugWidgetEnable = false;

    	UndoHistory m_undoHistory;
    	bool m_ctrlUndoWasDown = false;
    	bool m_ctrlRedoWasDown = false;

    	void refreshSelectionAfter(Command* command);

    	// Errors
    	std::string m_lastLoadSceneFileErrorMessage;
    	std::string m_lastLoadSceneErrorMessage;
    	std::string m_lastNFDOpenError;
    	std::string m_lastOpenProjectNFDError;
    	std::string m_lastSaveSceneNFDError;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_EDITORLAYER_H