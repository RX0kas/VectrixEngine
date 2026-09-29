#ifndef VECTRIXWORKSPACE_EDITORLAYER_H
#define VECTRIXWORKSPACE_EDITORLAYER_H
#include <functional>
#include <optional>

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
    	void OnDetach() override;
    	void openScene(const std::filesystem::path& path);
    	std::shared_ptr<Scene> getActiveScene() { return m_activeScene; }
	private:
    	void newScene(); ///< Replaces the scene with an empty one, saved to a file on its first save
    	void replaceActiveScene(std::shared_ptr<Scene> scene); ///< Hierarchy and undo history follow; file fields aren't set
    	bool saveScene(); ///< Saves to the scene's file, or asks for one (Save As) when it has none. False when not saved
    	bool showSaveDialog(); ///< Save As. False when cancelled or when the save failed
    	bool writeScene(const std::filesystem::path& path); ///< Reports a failure in a popup; on success the scene is clean
    	/// Runs action, which replaces or closes the scene, right away when nothing is unsaved; asks first otherwise
    	void runDiscardingScene(std::function<void()> action);
    	void renderUnsavedChangesPopup();
    	void updateWindowTitle(); ///< Scene file name, with a '*' while it has unsaved changes
    	/// Follows a file or folder the content browser moved or renamed: loaded assets, the open scene and every
    	/// scene file of the project are updated to the new paths
    	void onAssetMoved(const std::filesystem::path& from, const std::filesystem::path& to);
    	/// Updates the open scene's missing references and every scene file of the project from oldId to newId
    	/// @return The scenes that couldn't be updated, one "\n  - scene (reason)" line each
    	std::string remapAssetInProject(const std::string& oldId, const std::string& newId);
    	/// Finds the open scene's missing assets that were moved or renamed outside the editor and relinks them. Those
    	/// found by content update the project's scenes; those found by name only are a guess, kept to the open scene
    	/// and marked unsaved. @return One "\n  - old -> new" line per asset found again, and a note on the guesses
    	/// @param failures Receives the scenes that couldn't be updated
    	std::string relinkMovedAssets(std::string& failures);
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
    	bool m_ctrlSaveWasDown = false;
    	bool m_ctrlNewWasDown = false;

    	// Unsaved changes
    	std::function<void()> m_actionAfterSavePrompt; ///< What the "Unsaved changes" popup was opened for
    	bool m_openUnsavedChangesPopup = false; ///< Opened from OnImGuiRender, where its BeginPopupModal is
    	std::string m_baseWindowTitle; ///< The title before this layer, restored by OnDetach
    	std::optional<bool> m_titleModified; ///< What the title was last built from, nullopt before it first is
    	std::string m_titleFileName;

    	void refreshSelectionAfter(Command* command);

    	// Errors
    	std::string m_lastLoadSceneFileErrorMessage;
    	std::string m_lastLoadSceneErrorMessage;
    	std::string m_lastNFDOpenError;
    	std::string m_lastOpenProjectNFDError;
    	std::string m_lastSaveSceneNFDError;
    	std::string m_lastSaveSceneErrorMessage;
    	std::string m_lastSceneAssetsMissingMessage;
    	std::string m_lastAssetMoveErrorMessage;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_EDITORLAYER_H