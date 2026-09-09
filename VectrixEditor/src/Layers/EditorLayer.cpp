#include "EditorLayer.h"

#include "imgui.h"
#include "Vectrix/Scene/Components/CameraComponent.h"
#include "Utils/Gizmo.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/component_wise.hpp>

#include <nfd.h>

#include "Vectrix/Rendering/Camera/EditorCamera.h"
#include "Vectrix/Events/EditorEvent.h"
#include "Vectrix/Rendering/GraphicsContext.h"
#include "Vectrix/Settings/Outline.h"
#include "Vectrix/Settings/SettingsManager.h"

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

    	// Field of view and clip planes are fixed at construction (not live).
    	m_camera = std::make_unique<EditorCamera>(
    		static_cast<float>(settingNum({"editor", "camera", "fov"}, 50.0)),
    		static_cast<float>(settingNum({"editor", "camera", "near"}, 0.1)),
    		static_cast<float>(settingNum({"editor", "camera", "far"}, 1000.0)));
    	m_sceneHierarchyPanel.setContext(m_activeScene);

    	applyLiveSettings();

    	m_settingPanel.disable(); // opt-in via the Window menu
    }

	void EditorLayer::applyLiveSettings() {
    	m_cameraMoveSpeed      = static_cast<float>(settingNum({"editor", "camera", "moveSpeed"}, 1.5));
    	m_cameraRotationSpeed  = static_cast<float>(settingNum({"editor", "camera", "rotationSpeed"}, 50.0));
    	m_gizmoTranslationSnap = static_cast<float>(settingNum({"editor", "gizmo", "translationSnap"}, 0.5));
    	m_gizmoRotationSnap    = static_cast<float>(settingNum({"editor", "gizmo", "rotationSnap"}, 45.0));

    	glm::vec4 clearColor{0.0f, 0.0f, 0.0f, 1.0f};
    	if (const JsonObject& s = SettingsManager::getSettings(); s.contains("engine")) {
    		const JsonValue& c = s.at("engine")["rendering"]["clearColor"];
    		for (std::size_t i = 0; i < 4 && i < c.size(); ++i)
    			clearColor[static_cast<glm::length_t>(i)] =
    				static_cast<float>(c[i].getAs<double>().value_or(clearColor[static_cast<glm::length_t>(i)]));
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
    	SceneCreationData sceneCreationData = SceneSerializer::loadSceneFile(path.string());
    	if (sceneCreationData.result != SUCCESS) {
    		VC_CORE_ERROR_NO_EXIT("Error while loading scene file {}: {}",m_pendingScenePath,toString(sceneCreationData.result));
    		return;
    	}

    	auto newScene = Scene::loadScene(sceneCreationData);
    	if (newScene.first != SUCCESS) {
    		VC_CORE_ERROR_NO_EXIT("Error while loading scene {}: {}",m_pendingScenePath,toString(newScene.first));
    		return;
    	}

    	GraphicsContext::waitIdle();

    	m_activeScene->m_entities.clear();
    	m_activeScene->m_registry.clear<>();

    	m_activeScene = newScene.second;
    	m_activeScene->m_projectDirectory = path.parent_path();
    	m_activeScene->m_fileName = path.filename();
    	m_sceneHierarchyPanel.setContext(m_activeScene);

    	const std::filesystem::path settingsPath =
    		std::filesystem::path(m_activeScene->getProjectDirectory()) / settingsFileName;
    	if (const auto [settingsResult, settingsMessage] = Application::getSettingsManager().loadProject(settingsPath);
    		settingsResult != SUCCESS)
    		VC_CORE_WARN("Project settings not loaded ({}): {}", settingsPath.string(), settingsMessage);

    	const JsonObject& settings = SettingsManager::getSettings();
    	if (const auto editorNode = settings.find("editor"); editorNode != settings.end())
    		readOutlineSettings(editorNode->second["outline"]);

    	AssetsManager::instance().getMeshManager().clear();
    	AssetsManager::instance().getTextureManager().clear();
    	AssetsManager::instance().getMeshManager().clear();

    	GraphicsContext::waitIdle();
	}

	void EditorLayer::OnEvent(Event &event) {
    	if (event.getEventType()==EventType::WindowResize)
    		m_camera->recalculateMatrices();
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
    		VC_CORE_INFO("User cancelled");
    	} else {
    		VC_CORE_CRITICAL("NFD Error: {}", NFD_GetError());
    	}

    	NFD_Quit();
    }

	void EditorLayer::processPendingSceneLoad() {
    	if (m_pendingScenePath.empty())
    		return;

    	std::string path = m_pendingScenePath;
    	m_pendingScenePath.clear();

    	openScene(path);
	}

	void EditorLayer::showSaveDialog() {
    	NFD_Init();

    	nfdchar_t* outPath;
    	nfdfilteritem_t filters[] = { { "Vectrix Scene", "vctx" } };

    	nfdresult_t result = NFD_SaveDialog(&outPath, filters, 1, nullptr, "scene.vctx");

    	if (result == NFD_OKAY) {
    		const std::filesystem::path scenePath(outPath);
    		m_activeScene->m_projectDirectory = scenePath.parent_path();
    		m_activeScene->m_fileName = scenePath.filename();
    		SceneSerializer::saveScene(scenePath.string(),*m_activeScene);
    		NFD_FreePath(outPath);
    	} else if (result == NFD_CANCEL) {
    		VC_CORE_INFO("User cancelled");
    	} else {
    		VC_CORE_CRITICAL("NFD Error: {}", NFD_GetError());
    	}

    	NFD_Quit();
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
				if (ImGui::MenuItem("Open")) {
					showOpenDialog();
				}
				if (ImGui::MenuItem("Save")) {
					if (m_activeScene->getFileName().empty()) {
						showSaveDialog();
					} else {
						const std::filesystem::path scenePath =
							std::filesystem::path(m_activeScene->getProjectDirectory()) / m_activeScene->getFileName();
						SceneSerializer::saveScene(scenePath.string(),*m_activeScene);
					}
				}
				if (ImGui::MenuItem("Save As")) showSaveDialog();

				if (ImGui::MenuItem("Exit")) Application::instance().close();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window")) {
				if (ImGui::MenuItem("Graphics Debug", nullptr, m_graphicDebugWidgetEnable)) m_graphicDebugWidgetEnable = !m_graphicDebugWidgetEnable;
				ImGui::MenuItem("Settings (WIP)", nullptr, &m_settingPanel.getEnable());

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
			useGizmo(m_sceneHierarchyPanel.getSelectedEntity(),*m_camera,m_gizmoType,{m_viewportPos.x, m_viewportPos.y},{m_viewportSize.x,m_viewportSize.y},m_gizmoTranslationSnap,m_gizmoRotationSnap);
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE")) {
#ifdef VC_PLATFORM_LINUX
					const char* path = static_cast<const char *>(payload->Data);
#else
					const auto* path = static_cast<const wchar_t *>(payload->Data);
#endif
					openScene(AssetsManager::getAssetsPath()/path);
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
				m_sceneHierarchyPanel.setSelectedEntity(picked);
			else
				m_sceneHierarchyPanel.resetSelectedEntity();
		}

    	if (m_graphicDebugWidgetEnable) {
    		Application::instance().imguiLayer().getManager().renderDebugGraphicWidget(m_graphicDebugWidgetEnable);
    	}
    }

    void EditorLayer::OnRender() {

    }

    void EditorLayer::OnRenderOffscreen() {
        m_framebuffer->bind();

    	Renderer::beginScene(*m_camera);
    	m_activeScene->OnRender();
    	Renderer::endScene();
        m_framebuffer->unbind();
    	Renderer::renderOutline(m_sceneHierarchyPanel.getSelectedEntity(), m_framebuffer);
    }

    void EditorLayer::OnUpdate(const DeltaTime &dt) {
    	applyLiveSettings();
    	processPendingSceneLoad();

    	if (m_viewportFocused || m_viewportHovered) {
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

    		if (glm::length(moveDir) > 0.0f) {
    			moveDir = glm::normalize(moveDir);

    			glm::vec3 delta = (right * moveDir.x + forward * moveDir.z + up * moveDir.y) * m_cameraMoveSpeed * dt.getSeconds();

    			m_camera->m_position = m_camera->m_position + delta;
    		}

    		if (Input::isKeyPressed(VC_KEY_F)) {
    			auto e = m_sceneHierarchyPanel.getSelectedEntity();
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


    	if (Input::isKeyPressed(VC_KEY_Z))
    		m_gizmoType = -1;
    	if (Input::isKeyPressed(VC_KEY_X))
    		m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
    	if (Input::isKeyPressed(VC_KEY_C))
    		m_gizmoType = ImGuizmo::OPERATION::ROTATE;
    	if (Input::isKeyPressed(VC_KEY_V))
    		m_gizmoType = ImGuizmo::OPERATION::SCALE;

    	m_camera->recalculateMatrices();
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
