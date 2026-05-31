#include "EditorLayer.h"

#include "Vectrix/Scene/Components/CameraComponent.h"
#include "Utils/Gizmo.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/component_wise.hpp>

namespace Vectrix {
    EditorLayer::EditorLayer() : Layer("VC_Editor"), m_viewportSize(1, 1), m_camera(nullptr) {}

	void EditorLayer::OnAttach() {
    	FramebufferSpecification fbSpec;
    	fbSpec.width = 1;
    	fbSpec.height = 1;
    	m_framebuffer = Framebuffer::create(fbSpec);

    	m_activeScene = std::make_shared<Scene>();
    	m_cameraEntity = m_activeScene->createEntity("Camera");
    	m_camera = &m_cameraEntity.addComponent<CameraComponent>().camera;

    	ShaderUniformLayout layout;
    	m_viewportShader = ShaderManager::createShader("VC_viewport", "./shaders/viewport.vert", "./shaders/viewport.frag", layout);
    	m_testTexture = TextureManager::createTexture("VC_testTexture", "./textures/fox.png");

    	m_foxEntity = m_activeScene->createEntity("Fox");
    	m_foxEntity.addComponent<MeshRendererComponent>("./models/fox.obj", m_viewportShader, m_testTexture);
    	m_sceneHierarchyPanel.setContext(m_activeScene);
    }

    void EditorLayer::OnEvent(Event &event) {
    	m_camera->recalculateMatrices();
    }

    void EditorLayer::OnImGuiRender() {
        static bool dockingEnabled = true;
		if (dockingEnabled) {
			static bool dockspaceOpen = true;
			static bool opt_fullscreen_persistant = true;
			bool opt_fullscreen = opt_fullscreen_persistant;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

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

			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					// Disabling fullscreen would allow the window to be moved to the front of other windows,
					// which we can't undo at the moment without finer window depth/z control.
					//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

					if (ImGui::MenuItem("Exit")) Vectrix::Application::instance().close();
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
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
				m_viewportPos = {ImGui::GetWindowPos().x,ImGui::GetWindowPos().y};
				m_viewportPos.y += ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
				useGizmo(m_sceneHierarchyPanel.getSelectedEntity(),m_cameraEntity,m_gizmoType,{m_viewportPos.x, m_viewportPos.y},{m_viewportSize.x,m_viewportSize.y});
			}
			ImGui::End();
			ImGui::PopStyleVar();

			m_sceneHierarchyPanel.onImGuiRender();

			if (ImGui::IsMouseClicked(0) && m_viewportHovered && !ImGuizmo::IsOver()) {
				auto [mx, my] = ImGui::GetMousePos();
				Entity picked = pickEntity({ mx, my });
				if (picked)
					m_sceneHierarchyPanel.setSelectedEntity(picked);
				else
					m_sceneHierarchyPanel.resetSelectedEntity();
			}
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
    }

    void EditorLayer::OnUpdate(const DeltaTime &dt) {
    	if (m_viewportFocused || m_viewportHovered) {
    		glm::vec3 cameraRot = m_cameraEntity.getComponent<TransformComponent>().getRotationDeg();
    		if (Input::isKeyPressed(VC_KEY_LEFT))
    			cameraRot.y -= m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_RIGHT))
    			cameraRot.y += m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_DOWN))
    			cameraRot.x -= m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_UP))
    			cameraRot.x += m_cameraRotationSpeed * dt;
    		m_cameraEntity.getComponent<TransformComponent>().setRotationDeg(cameraRot);

    		glm::quat q = m_cameraEntity.getComponent<TransformComponent>().rotation;

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

    			m_cameraEntity.getComponent<TransformComponent>().position = m_cameraEntity.getComponent<TransformComponent>().position + delta;
    		}
    	}
    	if (m_mustResize) {
    		m_framebuffer->resize(m_viewportSize);
    		m_camera->setCustomAspect(m_viewportSize.x/m_viewportSize.y);
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
    	auto& camera = m_cameraEntity.getComponent<CameraComponent>().camera;
    	glm::mat4 invVP = glm::inverse(camera.getTransformationMatrix());
    	glm::vec4 rayClip  = { ndc.x, ndc.y, -1.0f, 1.0f };
    	glm::vec4 rayWorld = invVP * rayClip;
    	rayWorld /= rayWorld.w;

    	glm::vec3 rayOrigin = m_cameraEntity.getComponent<TransformComponent>().position;
    	glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld) - rayOrigin);
    	return rayDirection;
    }

	Entity EditorLayer::pickEntity(glm::vec2 mousePos) {
    	glm::vec3 rayDir = screenToWorldRay(mousePos);
    	glm::vec3 rayOrigin = m_cameraEntity.getComponent<TransformComponent>().position;

    	Entity closest = Entity::nullEntity();
    	float closestT = std::numeric_limits<float>::max();

    	auto view = m_activeScene->m_registry.view<TransformComponent, MeshRendererComponent>();
    	for (auto entityID : view) {
    		Entity entity{ entityID, m_activeScene.get() };
    		auto& tc = entity.getComponent<TransformComponent>();
    		auto& mc = entity.getComponent<MeshRendererComponent>();
    		float t;

    		glm::mat4 invModel = glm::inverse(tc.modelMatrix());
    		invModel[1][1] *= -1;
    		glm::vec3 localOrigin =	glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
    		glm::vec3 localDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));

    		if (mc.aabb.intersect(localOrigin, localDir, t)) {
    			if (t < closestT) {
    				closestT = t;
    				closest  = entity;
    			}
    		}
    	}

    	return closest;
    }
} // Vectrix