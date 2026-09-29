#include "EditorLayer.h"

#include "imgui.h"
#include "Vectrix/Scene/Components/CameraComponent.h"
#include "Utils/Gizmo.h"
#include "Undo/Commands.h"
#include "Undo/EntityHandleCommand.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/component_wise.hpp>

#include <nfd.h>

#include "StartupLayer.h"
#include "Utils/Error.h"
#include "Vectrix/Rendering/Camera/EditorCamera.h"
#include "Vectrix/Rendering/GraphicsContext.h"
#include "Vectrix/Settings/Outline.h"
#include "Vectrix/Settings/SettingsManager.h"
#include "Vectrix/Utils/Path.h"

namespace Vectrix {
    namespace {
        // Walk the in-memory settings tree without inserting: a missing link falls back to def.
        double settingNum(std::initializer_list<const char*> path, double def) {
            const JsonObject& root = SettingsManager::getSettings();
            auto it = path.begin();
            const auto found = root.find(*it);
            if (found == root.end()) return def;
            const JsonValue* cur = &found->second;
            for (++it; it != path.end(); ++it)
                cur = &(*cur)[*it];
            return cur->getAs<double>().value_or(def);
        }

        std::string settingStr(std::initializer_list<const char*> path, const std::string& def) {
            const JsonObject& root = SettingsManager::getSettings();
            auto it = path.begin();
            const auto found = root.find(*it);
            if (found == root.end()) return def;
            const JsonValue* cur = &found->second;
            for (++it; it != path.end(); ++it)
                cur = &(*cur)[*it];
            return cur->getAs<std::string>().value_or(def);
        }
    }

    EditorLayer::EditorLayer() : Layer("VC_Editor"), m_viewportSize(1, 1) {}

	void EditorLayer::OnAttach() {
    	FramebufferSpecification fbSpec;
    	fbSpec.width = 1;
    	fbSpec.height = 1;
    	m_framebuffer = Framebuffer::create(fbSpec);

    	m_activeScene = std::make_shared<Scene>("EditorScene");

    	// Field of view and clip planes are fixed at construction (not live)
    	m_camera = std::make_unique<EditorCamera>(
    		static_cast<float>(settingNum({"editor", "camera", "fov"}, 50.0)),
    		static_cast<float>(settingNum({"editor", "camera", "near"}, 0.1)),
    		static_cast<float>(settingNum({"editor", "camera", "far"}, 1000.0)));
    	m_sceneHierarchyPanel = std::make_unique<SceneHierarchyPanel>();
    	m_contentBrowserPanel = std::make_unique<ContentBrowserPanel>(AssetsManager::getAssetsPath());
    	m_contentBrowserPanel->setOnMoved([this](const std::filesystem::path& from, const std::filesystem::path& to) { onAssetMoved(from, to); });
    	m_settingPanel = std::make_unique<SettingsPanel>();
    	m_sceneHierarchyPanel->setContext(m_activeScene);
    	m_sceneHierarchyPanel->setUndoHistory(&m_undoHistory);

    	applyLiveSettings();

    	m_settingPanel->disable(); // opt-in via the Window menu
    	m_baseWindowTitle = Application::instance().window().getTitle();
    }

	void EditorLayer::OnDetach() {
    	Application::instance().window().setTitle(m_baseWindowTitle);
    }

	void EditorLayer::OnAttach(const JsonObject& data) {
    	if (data.contains("projectDirectory")) {
    		if (const auto projectDirectory = data.at("projectDirectory").getAs<std::string>()) {
    			m_projectDirectory = fromUtf8(*projectDirectory);
    			AssetsManager::setAssetsPath(m_projectDirectory / "Assets");
    		}
    	}
    	OnAttach();
    	if (data.contains("scenePath")) {
    		if (const auto scenePath = data.at("scenePath").getAs<std::string>())
    			openScene(fromUtf8(*scenePath));
    	}
    }

	void EditorLayer::refreshSelectionAfter(Command* command) {
    		if (auto* entityHandle = dynamic_cast<EntityHandleCommand*>(command))
    			m_sceneHierarchyPanel->setSelectedEntity(entityHandle->currentHandle());
    	}

	void EditorLayer::applyLiveSettings() {
		m_cameraMoveSpeed = static_cast<float>(settingNum({"editor", "camera", "moveSpeed"}, 1.5));
    	m_cameraRotationSpeed = static_cast<float>(settingNum({"editor", "camera", "rotationSpeed"}, 50.0));
    	m_gizmoTranslationSnap = static_cast<float>(settingNum({"editor", "gizmo", "translationSnap"}, 0.5));
    	m_gizmoRotationSnap = static_cast<float>(settingNum({"editor", "gizmo", "rotationSnap"}, 45.0));

    	glm::vec4 clearColor{0.0f, 0.0f, 0.0f, 1.0f};
    	if (const JsonObject& s = SettingsManager::getSettings(); s.contains("engine")) {
    		const JsonValue& c = s.at("engine")["rendering"]["clearColor"];
    		for (std::size_t i = 0; i < 4 && i < c.size(); ++i)
    			clearColor[static_cast<glm::length_t>(i)] = static_cast<float>(c[i].getAs<double>().value_or(clearColor[static_cast<glm::length_t>(i)]));
    	}
    	if (clearColor != m_appliedClearColor) {
    		m_appliedClearColor = clearColor;
    		RenderCommand::setClearColor(clearColor);
    	}

    	if (const std::string theme = settingStr({"editor", "ui", "theme"}, "dark"); theme != m_appliedTheme) {
    		m_appliedTheme = theme;
    		if (theme == "light") {
    			ImGui::StyleColorsLight();
    		} else {
    			ImGui::StyleColorsDark();
    			ImGuiLayer::setDarkThemeColors();
    		}
    	}
    }

	void EditorLayer::openScene(const std::filesystem::path& path) {
    	SceneCreationData sceneCreationData = SceneSerializer::loadSceneFile(toUtf8(path));
    	if (sceneCreationData.result != SUCCESS) {
    		showErrorMessage("ERROR_LOADING_SCENE_FILE");
    		m_lastLoadSceneFileErrorMessage = std::format("Error while loading scene file {}: {}",toUtf8(path),toString(sceneCreationData.result));
    		return;
    	}

    	std::vector<std::string> assetErrors;
    	auto newScene = Scene::loadScene(sceneCreationData, &assetErrors);
    	if (newScene.first != SUCCESS) {
    		showErrorMessage("ERROR_LOADING_SCENE");
    		m_lastLoadSceneErrorMessage = std::format("Error while loading scene {}: {}",toUtf8(path),toString(newScene.first));
    		return;
    	}

    	replaceActiveScene(newScene.second);
    	m_activeScene->m_directory = path.parent_path();
    	m_activeScene->m_fileName = toUtf8(path.filename());

    	// The project tier lives at the project root, not next to the scene (scenes are in <project>/Scenes):
    	// using the scene's folder read and wrote <project>/Scenes/settings.vectrix.json instead
    	const std::filesystem::path settingsPath = m_projectDirectory.empty() ? std::filesystem::path{} : m_projectDirectory / settingsFileName;
    	if (const auto [settingsResult, settingsMessage] = Application::getSettingsManager().loadProject(settingsPath); settingsResult != SUCCESS)
    		VC_WARN("Project settings not loaded ({}): {}", toUtf8(settingsPath), settingsMessage);

    	const JsonObject& settings = SettingsManager::getSettings();
    	if (const auto editorNode = settings.find("editor"); editorNode != settings.end())
    		readOutlineSettings(editorNode->second["outline"]);

    	AssetsManager::instance().getMeshManager().clear();
    	AssetsManager::instance().getTextureManager().clear();

    	GraphicsContext::waitIdle();

    	if (assetErrors.empty())
    		return;

    	// Missing assets may have been moved or renamed outside the editor: found again by their content, or failing
    	// that by their name
    	std::string moveFailures;
    	const std::string relinked = relinkMovedAssets(moveFailures);

    	// The others: the scene opened without them, but keeps their paths (MeshRendererComponent::missingMesh...)
    	std::string stillMissing;
    	for (const auto& [handle, entity] : m_activeScene->m_entities) {
    		if (!entity->hasComponent<MeshRendererComponent>())
    			continue;
    		const auto& mc = entity->getComponent<MeshRendererComponent>();
    		for (const std::string* missing : {&mc.missingMesh, &mc.missingTexture, &mc.missingShader}) {
    			if (!missing->empty())
    				stillMissing += std::format("\n  - {}: {}", entity->getComponent<InformationComponent>().name, *missing);
    		}
    	}

    	std::string message;
    	if (!stillMissing.empty())
    		message = std::format("{} opened without these assets:{}\n\nThey stay in the scene, saving included: put the files back and reopen it, or replace them in the Mesh Renderer.",
    			toUtf8(path.filename()), stillMissing);
    	if (!relinked.empty())
    		message += std::format("{}These assets were moved or renamed outside the editor and found again (the project's scenes now use their new paths):{}",
    			message.empty() ? "" : "\n\n", relinked);
    	if (!moveFailures.empty())
    		message += std::format("\n\nThese scenes couldn't be updated and still use the old paths:{}", moveFailures);
    	showErrorMessage("SCENE_ASSETS_MISSING");
    	m_lastSceneAssetsMissingMessage = std::move(message);
	}

	void EditorLayer::replaceActiveScene(std::shared_ptr<Scene> scene) {
    	GraphicsContext::waitIdle();

    	m_activeScene->m_entities.clear();
    	m_activeScene->m_registry.clear<>();

    	m_activeScene = std::move(scene);
    	m_sceneHierarchyPanel->setContext(m_activeScene);
    	m_undoHistory.clear();
    }

	void EditorLayer::newScene() {
    	replaceActiveScene(std::make_shared<Scene>("Untitled"));
    	// No file yet: the first save asks for one, starting in the project's scene folder
    	if (!m_projectDirectory.empty())
    		m_activeScene->m_directory = m_projectDirectory / "Scenes";
    }

	void EditorLayer::onAssetMoved(const std::filesystem::path& from, const std::filesystem::path& to) {
    	std::error_code ec;
    	const auto normal = [&ec](const std::filesystem::path& path) { return std::filesystem::absolute(path, ec).lexically_normal(); };

    	// The open scene follows its own file when that is what moved (a scene saved in the assets folder)
    	if (!m_activeScene->m_fileName.empty()) {
    		const std::filesystem::path sceneFile = normal(m_activeScene->m_directory / fromUtf8(m_activeScene->m_fileName));
    		const std::filesystem::path relative = sceneFile.lexically_relative(normal(from));
    		if (!relative.empty() && *relative.begin() != "..") {
    			const std::filesystem::path movedScene = relative == "." ? normal(to) : normal(to) / relative;
    			m_activeScene->m_directory = movedScene.parent_path();
    			m_activeScene->m_fileName = toUtf8(movedScene.filename());
    		}
    	}

    	const std::optional<std::string> oldId = AssetsManager::getProjectAssetId(from);
    	const std::optional<std::string> newId = AssetsManager::getProjectAssetId(to);
    	if (!oldId || !newId)
    		return;

    	// Loaded assets keep their objects under the new ids: the open scene saves the new paths
    	AssetsManager::instance().moveProjectAssets(*oldId, *newId);

    	if (const std::string failures = remapAssetInProject(*oldId, *newId); !failures.empty()) {
    		showErrorMessage("ASSET_MOVE_SCENES_ERROR");
    		m_lastAssetMoveErrorMessage = std::format("{} moved to {}, but these scenes couldn't be updated and still use the old path:{}", *oldId, *newId, failures);
    	}
    }

	std::string EditorLayer::remapAssetInProject(const std::string& oldId, const std::string& newId) {
    	const auto remap = [&](const std::string& id) { return AssetsManager::remapAssetId(id, oldId, newId); };
    	for (const auto& [handle, entity] : m_activeScene->m_entities) {
    		if (!entity->hasComponent<MeshRendererComponent>())
    			continue;
    		auto& mc = entity->getComponent<MeshRendererComponent>();
    		for (std::string* missing : {&mc.missingMesh, &mc.missingTexture, &mc.missingShader}) {
    			if (!missing->empty())
    				if (std::optional<std::string> moved = remap(*missing))
    					*missing = std::move(*moved);
    		}
    	}

    	// Every scene file of the project, the open scene's included. Listed first: remapAssetIds replaces files
    	std::error_code ec;
    	std::vector<std::filesystem::path> scenes;
    	if (!m_projectDirectory.empty()) {
    		for (auto it = std::filesystem::recursive_directory_iterator(m_projectDirectory, std::filesystem::directory_options::skip_permission_denied, ec);
    			 !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
    			if (it->is_regular_file(ec) && it->path().extension() == ".vctx")
    				scenes.push_back(it->path());
    		}
    	}

    	int updated = 0;
    	std::string failures;
    	for (const std::filesystem::path& scene : scenes) {
    		const auto [result, rewritten] = SceneSerializer::remapAssetIds(toUtf8(scene), remap);
    		if (result != SUCCESS)
    			failures += std::format("\n  - {} ({})", toUtf8(scene.lexically_relative(m_projectDirectory)), toString(result));
    		else if (rewritten)
    			++updated;
    	}
    	VC_INFO("Moved {} to {}: {} scene file(s) updated", oldId, newId, updated);
    	return failures;
    }

	std::string EditorLayer::relinkMovedAssets(std::string& failures) {
    	// Searched once per missing id, even when several entities use it
    	std::unordered_map<std::string, std::optional<AssetsManager::MovedAsset>> found;
    	std::string relinked;
    	bool relinkedByNameOnly = false;
    	const auto relink = [&](auto& asset, std::string& missing) {
    		using T = typename std::remove_reference_t<decltype(asset)>::element_type;
    		if (missing.empty())
    			return;
    		std::error_code ec;
    		if (std::filesystem::exists(AssetsManager::getAssetsPath() / fromUtf8(missing), ec))
    			return; // there but broken: nothing moved

    		auto it = found.find(missing);
    		if (it == found.end()) {
    			const auto fingerprint = m_activeScene->m_missingFingerprints.find(missing);
    			std::optional<AssetsManager::MovedAsset> moved = AssetsManager::findMovedAsset(missing,
    				fingerprint != m_activeScene->m_missingFingerprints.end() ? fingerprint->second : AssetFingerprint{});
    			if (moved)
    				relinked += std::format("\n  - {} -> {}{}", missing, moved->id, moved->byNameOnly ? " (by name only)" : "");
    			it = found.emplace(missing, std::move(moved)).first;
    		}
    		if (!it->second)
    			return;

    		auto [result, loaded] = AssetsManager::load<T>(it->second->id);
    		if (result != SUCCESS)
    			return;
    		asset = loaded;
    		missing.clear();
    		relinkedByNameOnly |= it->second->byNameOnly;
    	};

    	for (const auto& [handle, entity] : m_activeScene->m_entities) {
    		if (!entity->hasComponent<MeshRendererComponent>())
    			continue;
    		auto& mc = entity->getComponent<MeshRendererComponent>();
    		relink(mc.mesh, mc.missingMesh);
    		relink(mc.texture, mc.missingTexture);
    		relink(mc.shader, mc.missingShader);
    		// Kept disabled only because of them: drawn again, as it was saved
    		if (mc.enabledOnceComplete && !mc.hasMissingAsset() && mc.tryEnabling())
    			mc.enabledOnceComplete = false;
    	}

    	// Found by content: like a move made in the content browser, every scene of the project follows, this one's
    	// file included. Found by name only, it's a guess: only the open scene uses it, and only once saved
    	for (const auto& [oldId, moved] : found) {
    		if (!moved || moved->byNameOnly)
    			continue;
    		m_activeScene->m_missingFingerprints.erase(oldId);
    		failures += remapAssetInProject(oldId, moved->id);
    	}
    	if (relinkedByNameOnly) {
    		m_undoHistory.markModified(); // the scene file still names the old paths
    		relinked += "\n\nThose marked \"by name only\" are the one file with that name, but not with the content the "
    			"scene was saved with: they may have been edited since, or be a different file. Check them before saving "
    			"the scene (other scenes are left as they are), and replace a wrong one in the Mesh Renderer.";
    	}
    	return relinked;
    }

	void EditorLayer::OnEvent(Event &event) {
    	if (event.getEventType()==EventType::WindowResize)
    		m_camera->recalculateMatrices();

    	// Handling the close request keeps the application running while the user is asked about unsaved changes
    	if (event.getEventType() == EventType::WindowClose && !m_undoHistory.isClean()) {
    		event.Handled = true;
    		runDiscardingScene([] { Application::instance().close(); });
    	}
    }

	void EditorLayer::showOpenDialog() {
    	NFD_Init();

    	nfdchar_t* outPath;
    	nfdfilteritem_t filters[] = { { "Vectrix Scene", "vctx" } };

    	nfdresult_t result = NFD_OpenDialog(&outPath, filters, 1, nullptr);

    	if (result == NFD_OKAY) {
    		m_pendingScenePath = std::string(outPath);
    		NFD_FreePath(outPath);
    	} else if (result == NFD_CANCEL) {
    		VC_INFO("User cancelled");
    	} else {
    		showErrorMessage("OPEN_DIALOG_NFD_ERROR");
    		m_lastNFDOpenError = std::format("NFD Error: {}", NFD_GetError());
    	}

    	NFD_Quit();
    }
	void EditorLayer::showOpenProjectDialog() {
	    NFD_Init();

    	nfdchar_t* outPath;
    	nfdfilteritem_t filters[] = { { "Vectrix Project", "vcproj" } };

    	nfdresult_t result = NFD_OpenDialog(&outPath, filters, 1, nullptr);

    	if (result == NFD_OKAY) {
    		m_pendingProjectPath = std::string(outPath);
    		NFD_FreePath(outPath);
    	} else if (result == NFD_CANCEL) {
    		VC_INFO("User cancelled");
    	} else {
    		showErrorMessage("OPEN_PROJECT_DIALOG_NFD_ERROR");
    		m_lastOpenProjectNFDError = std::format("NFD Error: {}", NFD_GetError());
    	}

    	NFD_Quit();
    }

	void EditorLayer::processPendingSceneLoad() {
    	if (m_pendingScenePath.empty())
    		return;

    	std::string path = m_pendingScenePath;
    	m_pendingScenePath.clear();

    	openScene(fromUtf8(path));
	}

	void EditorLayer::processPendingProjectLoad() {
    	if (m_pendingProjectPath.empty())
    		return;

    	std::string path = m_pendingProjectPath;
    	m_pendingProjectPath.clear();
		JsonObject data;
    	data.emplace("openProject",path); // the key StartupLayer::OnAttach looks for ("path" was ignored: nothing opened)
    	Application::instance().switchToLayer<StartupLayer>(this,data);
    }

	bool EditorLayer::showSaveDialog() {
    	NFD_Init();

    	nfdchar_t* outPath;
    	nfdfilteritem_t filters[] = { { "Vectrix Scene", "vctx" } };

    	// Starts where the scene is (or will be, for a new one), with its current or future file name
    	std::filesystem::path folder = m_activeScene->m_directory;
    	if (folder.empty() && !m_projectDirectory.empty())
    		folder = m_projectDirectory / "Scenes";
    	std::error_code ec;
    	const std::string defaultFolder = folder.empty() ? std::string{} : toUtf8(std::filesystem::absolute(folder, ec)); // NFD takes UTF-8
    	const std::string defaultName = !m_activeScene->m_fileName.empty() ? m_activeScene->m_fileName
    		: (m_activeScene->m_name.empty() ? std::string("Untitled") : m_activeScene->m_name) + ".vctx";

    	nfdresult_t result = NFD_SaveDialog(&outPath, filters, 1,
    		defaultFolder.empty() ? nullptr : defaultFolder.c_str(), defaultName.c_str());

    	bool saved = false;
    	if (result == NFD_OKAY) {
    		saved = writeScene(fromUtf8(outPath));
    		NFD_FreePath(outPath);
    	} else if (result == NFD_CANCEL) {
    		VC_INFO("User cancelled");
    	} else {
    		showErrorMessage("SAVE_DIALOG_NFD_ERROR");
    		m_lastSaveSceneNFDError = std::format("NFD Error: {}", NFD_GetError());
    	}

    	NFD_Quit();
    	return saved;
    }

	bool EditorLayer::saveScene() {
    	if (m_activeScene->m_fileName.empty())
    		return showSaveDialog();
    	return writeScene(m_activeScene->m_directory / fromUtf8(m_activeScene->m_fileName));
    }

	bool EditorLayer::writeScene(const std::filesystem::path& path) {
    	// A scene saved for the first time is named after its file, like a project's starting scene
    	std::string previousName = m_activeScene->m_name;
    	if (m_activeScene->m_fileName.empty())
    		m_activeScene->m_name = toUtf8(path.stem());

    	if (const VectrixResult result = SceneSerializer::saveScene(toUtf8(path), *m_activeScene); result != SUCCESS) {
    		m_activeScene->m_name = std::move(previousName);
    		showErrorMessage("SAVE_SCENE_ERROR");
    		m_lastSaveSceneErrorMessage = std::format("Can't save the scene to {}: {}", toUtf8(path), toString(result));
    		return false;
    	}
    	m_activeScene->m_directory = path.parent_path();
    	m_activeScene->m_fileName = toUtf8(path.filename());
    	m_undoHistory.markClean();
    	return true;
    }

	void EditorLayer::runDiscardingScene(std::function<void()> action) {
    	if (m_undoHistory.isClean()) {
    		action();
    		return;
    	}
    	// A later request replaces an unanswered one: the prompt is about whatever was asked last
    	m_actionAfterSavePrompt = std::move(action);
    	m_openUnsavedChangesPopup = true;
    }

	void EditorLayer::renderUnsavedChangesPopup() {
    	constexpr const char* popupId = "Unsaved changes";
    	if (m_openUnsavedChangesPopup) {
    		ImGui::OpenPopup(popupId);
    		m_openUnsavedChangesPopup = false;
    	}
    	if (!ImGui::BeginPopupModal(popupId, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    		return;

    	ImGui::Text("Save the changes made to %s?",
    		m_activeScene->m_fileName.empty() ? "this scene" : m_activeScene->m_fileName.c_str());
    	ImGui::TextDisabled("Without saving, they are lost.");
    	ImGui::Separator();

    	// Run after EndPopup: the action may replace the scene or leave this layer
    	std::function<void()> action;
    	if (ImGui::Button("Save")) {
    		// A failed save (reported in its own popup) or a cancelled Save As keeps the scene open
    		if (saveScene())
    			action = std::move(m_actionAfterSavePrompt);
    		m_actionAfterSavePrompt = nullptr;
    		ImGui::CloseCurrentPopup();
    	}
    	ImGui::SameLine();
    	if (ImGui::Button("Don't Save")) {
    		action = std::move(m_actionAfterSavePrompt);
    		m_actionAfterSavePrompt = nullptr;
    		ImGui::CloseCurrentPopup();
    	}
    	ImGui::SameLine();
    	if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
    		m_actionAfterSavePrompt = nullptr;
    		ImGui::CloseCurrentPopup();
    	}
    	ImGui::EndPopup();

    	if (action)
    		action();
    }

	void EditorLayer::updateWindowTitle() {
    	const bool modified = !m_undoHistory.isClean();
    	const std::string& fileName = m_activeScene->m_fileName;
    	if (m_titleModified == modified && m_titleFileName == fileName)
    		return; // unchanged: no string built every frame

    	m_titleModified = modified;
    	m_titleFileName = fileName;
    	Application::instance().window().setTitle(std::format("{} - {}{}", m_baseWindowTitle,
    		fileName.empty() ? "Untitled scene" : fileName, modified ? " *" : ""));
    }

    void EditorLayer::OnImGuiRender() {
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)	{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		// Menu Bar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					runDiscardingScene([this] { newScene(); });
				if (ImGui::MenuItem("Open"))
					runDiscardingScene([this] { showOpenDialog(); });
				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) saveScene();
				if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) showSaveDialog();

				if (ImGui::MenuItem("Open Project"))
					runDiscardingScene([this] { showOpenProjectDialog(); });

				if (ImGui::BeginMenu("Open recent project")) {
					// Cached by StartupLayer: only re-read from disk when recentProjects.json changed
					for (const RecentProject& project : StartupLayer::loadRecentProject()) {
						ImGui::PushID(&project);
						const bool clicked = ImGui::MenuItem(project.name.c_str());
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("%s", toUtf8(project.path).c_str());
						ImGui::PopID();
						if (clicked) {
							runDiscardingScene([this, projectPath = project.path] {
								JsonObject data;
								// toUtf8(), not c_str(): on Windows c_str() is a wchar_t* that would silently become a JsonValue(bool)
								data["openProject"] = JsonValue(toUtf8(projectPath));
								Application::instance().switchToLayer<StartupLayer>(this, data);
							});
							// The new StartupLayer may be constructed right away and reload the list we're iterating
							break;
						}
					}
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Close Project"))
					runDiscardingScene([this] { Application::instance().switchToLayer<StartupLayer>(this); });

				if (ImGui::MenuItem("Exit"))
					runDiscardingScene([] { Application::instance().close(); });
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo", "Ctrl+Z", false, m_undoHistory.canUndo()))
					refreshSelectionAfter(m_undoHistory.undo());
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, m_undoHistory.canRedo()))
					refreshSelectionAfter(m_undoHistory.redo());
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window")) {
				if (ImGui::MenuItem("Graphics Debug", nullptr, m_graphicDebugWidgetEnable)) m_graphicDebugWidgetEnable = !m_graphicDebugWidgetEnable;
				ImGui::MenuItem("Settings", nullptr, &m_settingPanel->getEnable());

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});
		ImGui::Begin("Viewport");
		m_viewportFocused = ImGui::IsWindowFocused();
		m_viewportHovered = ImGui::IsWindowHovered();
		if (!m_viewportFocused || !m_viewportHovered)
			Application::instance().imguiLayer().startBlockEvents();
		else
			Application::instance().imguiLayer().stopBlockEvents();

		ImVec2 size = ImGui::GetContentRegionAvail();
		if (size.x==0 || size.y==0) {
			ImGui::Text("Loading...");
		} else {
			if (size.x != m_viewportSize.x || size.y != m_viewportSize.y) {
				m_mustResize = true;
				m_viewportSize = {size.x,size.y};
			}
			ImGui::Image(m_framebuffer->getTextureID(),{m_viewportSize.x,m_viewportSize.y});
			m_viewportPos = changeVecType<glm::vec2,ImVec2,2>(ImGui::GetItemRectMin());
			useGizmo(m_sceneHierarchyPanel->getSelectedEntity(),*m_camera,m_gizmoType,{m_viewportPos.x, m_viewportPos.y},{m_viewportSize.x,m_viewportSize.y},m_undoHistory,m_gizmoTranslationSnap,m_gizmoRotationSnap);
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE")) {
#ifdef VC_PLATFORM_LINUX
					const char* path = static_cast<const char *>(payload->Data);
#else
					const auto* path = static_cast<const wchar_t *>(payload->Data);
#endif
					runDiscardingScene([this, scenePath = AssetsManager::getAssetsPath() / path] { openScene(scenePath); });
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

		if (ImGui::IsMouseClicked(0) && m_viewportHovered && !ImGuizmo::IsOver()) {
			auto [mx, my] = ImGui::GetMousePos();
			std::shared_ptr<Entity> picked = pickEntity({ mx, my });
			if (picked)
				m_sceneHierarchyPanel->setSelectedEntity(picked);
			else
				m_sceneHierarchyPanel->resetSelectedEntity();
		}

    	if (m_graphicDebugWidgetEnable) {
    		Application::instance().imguiLayer().getManager().renderDebugGraphicWidget(m_graphicDebugWidgetEnable);
    	}

    	renderErrorMessage("ERROR_LOADING_SCENE_FILE",m_lastLoadSceneFileErrorMessage);
    	renderErrorMessage("ERROR_LOADING_SCENE",m_lastLoadSceneErrorMessage);
    	renderErrorMessage("OPEN_DIALOG_NFD_ERROR", m_lastNFDOpenError);
    	renderErrorMessage("OPEN_PROJECT_DIALOG_NFD_ERROR", m_lastOpenProjectNFDError);
    	renderErrorMessage("SAVE_DIALOG_NFD_ERROR", m_lastSaveSceneNFDError);
    	renderErrorMessage("SAVE_SCENE_ERROR", m_lastSaveSceneErrorMessage);
    	renderErrorMessage("SCENE_ASSETS_MISSING", m_lastSceneAssetsMissingMessage);
    	renderErrorMessage("ASSET_MOVE_SCENES_ERROR", m_lastAssetMoveErrorMessage);
    	renderUnsavedChangesPopup();
    	renderErrorMessage("LOAD_RECENT_PROJECTS_ERROR", StartupLayer::lastLoadRecentProjectsError());
    }

    void EditorLayer::OnRender() {

    }

    void EditorLayer::OnRenderOffscreen() {
    	m_framebuffer->bind();

    	Renderer::beginScene(*m_camera);
    	m_activeScene->OnRender();
    	Renderer::endScene();

    	m_framebuffer->unbind();

    	Renderer::renderOutline(m_sceneHierarchyPanel->getSelectedEntity(), m_framebuffer);
    }

    void EditorLayer::OnUpdate(const DeltaTime &dt) {
    	applyLiveSettings();
    	processPendingSceneLoad();
    	processPendingProjectLoad();

    	const bool ctrlDown = Input::isKeyPressed(VC_KEY_LEFT_CONTROL) || Input::isKeyPressed(VC_KEY_RIGHT_CONTROL);
    	// The keys polled below are read from GLFW, which ImGui doesn't filter: while a text field is being typed
    	// in (an entity name, a search box), its letters must not move the camera or switch the gizmo
    	const bool typing = ImGui::GetIO().WantTextInput;

    	if ((m_viewportFocused || m_viewportHovered) && !typing) {
    		glm::vec3 cameraRot = m_camera->getRotationDeg();
    		if (Input::isKeyPressed(VC_KEY_LEFT))
    			cameraRot.y -= m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_RIGHT))
    			cameraRot.y += m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_DOWN))
    			cameraRot.x -= m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_UP))
    			cameraRot.x += m_cameraRotationSpeed * dt;
    		m_camera->setRotationDeg(cameraRot);

    		glm::quat q = m_camera->m_rotation;

    		glm::vec3 forward = q * glm::vec3(0, 0, -1);
    		glm::vec3 right = q * glm::vec3(1, 0, 0);
    		glm::vec3 up = q * glm::vec3(0, 1, 0);

    		glm::vec3 moveDir(0.0f);
    		if (Input::isKeyPressed(VC_KEY_A)) moveDir.x -= 1.0f;
    		if (Input::isKeyPressed(VC_KEY_D)) moveDir.x += 1.0f;
    		if (Input::isKeyPressed(VC_KEY_W)) moveDir.z += 1.0f;
    		if (Input::isKeyPressed(VC_KEY_S)) moveDir.z -= 1.0f;
    		if (Input::isKeyPressed(VC_KEY_Q)) moveDir.y -= 1.0f;
    		if (Input::isKeyPressed(VC_KEY_E)) moveDir.y += 1.0f;

    		// Not while Ctrl is held: those keys are then shortcuts (Ctrl+S would also move the camera back)
    		if (glm::length(moveDir) > 0.0f && !ctrlDown) {
    			moveDir = glm::normalize(moveDir);

    			glm::vec3 delta = (right * moveDir.x + forward * moveDir.z + up * moveDir.y) * m_cameraMoveSpeed * dt.getSeconds();

    			m_camera->m_position = m_camera->m_position + delta;
    		}

    		if (Input::isKeyPressed(VC_KEY_F)) {
    			auto e = m_sceneHierarchyPanel->getSelectedEntity();
    			if (e) {
    				m_camera->setViewTarget(e->getComponent<TransformComponent>().position);
    			}
    		}
    	}
    	if (m_mustResize) {
    		m_framebuffer->resize(m_viewportSize);
    		m_camera->setCustomAspect(m_viewportSize.x/m_viewportSize.y);
			Renderer::resizeMask(m_viewportSize);
    		m_mustResize = false;
    	}

    	m_activeScene->OnUpdate(dt);


    	bool zDown = Input::isKeyPressed(VC_KEY_Z);
    	bool yDown = Input::isKeyPressed(VC_KEY_Y);

    	// Not while typing: Ctrl+Z/Y then undo the text being edited, which the field does itself
    	if (ctrlDown && zDown && !m_ctrlUndoWasDown && !typing && m_undoHistory.canUndo())
    		refreshSelectionAfter(m_undoHistory.undo());
    	m_ctrlUndoWasDown = ctrlDown && zDown;

    	if (ctrlDown && yDown && !m_ctrlRedoWasDown && !typing && m_undoHistory.canRedo())
    		refreshSelectionAfter(m_undoHistory.redo());
    	m_ctrlRedoWasDown = ctrlDown && yDown;

    	const bool sDown = Input::isKeyPressed(VC_KEY_S);
    	if (ctrlDown && sDown && !m_ctrlSaveWasDown) {
    		if (Input::isKeyPressed(VC_KEY_LEFT_SHIFT) || Input::isKeyPressed(VC_KEY_RIGHT_SHIFT))
    			showSaveDialog();
    		else
    			saveScene();
    	}
    	m_ctrlSaveWasDown = ctrlDown && sDown;

    	const bool nDown = Input::isKeyPressed(VC_KEY_N);
    	if (ctrlDown && nDown && !m_ctrlNewWasDown)
    		runDiscardingScene([this] { newScene(); });
    	m_ctrlNewWasDown = ctrlDown && nDown;

    	// Bare keys only: with Ctrl they're undo, cut, copy and paste (the content browser's included)
    	if (!ctrlDown && !typing) {
    		if (zDown)
    			m_gizmoType = -1;
    		if (Input::isKeyPressed(VC_KEY_X))
    			m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
    		if (Input::isKeyPressed(VC_KEY_C))
    			m_gizmoType = ImGuizmo::OPERATION::ROTATE;
    		if (Input::isKeyPressed(VC_KEY_V))
    			m_gizmoType = ImGuizmo::OPERATION::SCALE;
    	}

    	m_camera->recalculateMatrices();
    	updateWindowTitle();
    }
	glm::vec3 EditorLayer::screenToWorldRay(glm::vec2 mousePos) {
		glm::vec2 ndc = {
			(2.0f * (mousePos.x - m_viewportPos.x) / m_viewportSize.x) - 1.0f,
			1.0f - (2.0f * (mousePos.y - m_viewportPos.y) / m_viewportSize.y)
		};
		glm::mat4 proj = m_camera->getProjectionMatrix();
		proj[1][1] *= -1.0f;
		glm::mat4 invVP = glm::inverse(proj * m_camera->getViewMatrix());
		glm::vec4 rayClip  = { ndc.x, ndc.y, -1.0f, 1.0f };
		glm::vec4 rayWorld = invVP * rayClip;
		rayWorld /= rayWorld.w;

		glm::vec3 rayOrigin = m_camera->m_position;
		glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld) - rayOrigin);
		return rayDirection;
	}

	std::shared_ptr<Entity> EditorLayer::pickEntity(glm::vec2 mousePos) {
		glm::vec3 rayDir = screenToWorldRay(mousePos);
		glm::vec3 rayOrigin = m_camera->m_position;

		std::shared_ptr<Entity> closest;
		float closestT = std::numeric_limits<float>::max();

		for (const auto& e : m_activeScene->m_entities) {
			std::shared_ptr<Entity> entity = e.second;

			if (entity->hasComponent<CameraComponent>()) { continue; } // TODO: make camera visible and clickable
			if (!entity->hasComponent<MeshRendererComponent>()) { continue; }

			auto& tc = entity->getComponent<TransformComponent>();
			auto& mc = entity->getComponent<MeshRendererComponent>();
			if (!mc.isEnable() || !mc.mesh) { continue; }

			float t = 0;
			glm::mat4 modelMatrix = tc.modelMatrix();
			glm::mat4 invModel = glm::inverse(modelMatrix);
			glm::vec3 localOrigin =	glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
			glm::vec3 localDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));

			if (mc.mesh->getAABB().intersect(localOrigin, localDir, t)) {
				if (t < closestT) {
					closestT = t;
					closest = entity;
				}
			}
		}

		return closest;
	}
} // Vectrix
