#include "EditorLayer.h"

#include "Vectrix/Scene/Components/CameraComponent.h"

namespace Vectrix {
    EditorLayer::EditorLayer() : Layer("VC_Editor"), m_viewportSize(1, 1) {
	    FramebufferSpecification fbSpec;
	    fbSpec.width = 1;
	    fbSpec.height = 1;
	    m_framebuffer = Framebuffer::create(fbSpec);

	    m_activeScene = std::make_shared<Scene>();
	    m_cameraEntity = m_activeScene->createEntity();
	    m_cameraEntity.addComponent<CameraComponent>(m_cameraEntity.getComponent<TransformComponent>());
    	m_camera = Camera::getCurrentCamera();

	    ShaderUniformLayout layout;
	    m_viewportShader = ShaderManager::createShader("VC_viewport", "./shaders/viewport.vert", "./shaders/viewport.frag", layout);
	    m_testTexture = TextureManager::createTexture("VC_testTexture", "./textures/fox.png");

	    m_foxEntity = m_activeScene->createEntity();
	    m_foxEntity.addComponent<MeshComponent>("./models/fox.obj", m_viewportShader, m_testTexture);
    }

    void EditorLayer::OnEvent(Event &event) {
    	m_camera->recalculateMatrices();
    }

    void EditorLayer::OnImGuiRender() {
        static bool dockingEnabled = false;
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
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}
    }

    void EditorLayer::OnRender() {
    	Renderer::beginScene(*m_camera);
    	m_activeScene->OnRender();
    	Renderer::endScene();
    }

    void EditorLayer::OnRenderOffscreen() {
        m_framebuffer->bind();

        m_framebuffer->unbind();
    }

    void EditorLayer::OnUpdate(const DeltaTime &dt) {
    	if (m_viewportFocused || m_viewportHovered || true) {
    		glm::vec3 cameraRot = m_cameraEntity.getComponent<TransformComponent>().rotation;
    		if (Input::isKeyPressed(VC_KEY_LEFT))
    			cameraRot.y -= m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_RIGHT))
    			cameraRot.y += m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_UP))
    			cameraRot.x -= m_cameraRotationSpeed * dt;
    		if (Input::isKeyPressed(VC_KEY_DOWN))
    			cameraRot.x += m_cameraRotationSpeed * dt;
    		m_cameraEntity.getComponent<TransformComponent>().rotation = cameraRot;

    		float yaw = cameraRot.y;
    		float pitch = cameraRot.x;
    		float roll = cameraRot.z;

    		float c1 = std::cos(yaw);
    		float s1 = std::sin(yaw);
    		float c2 = std::cos(pitch);
    		float s2 = std::sin(pitch);
    		float c3 = std::cos(roll);
    		float s3 = std::sin(roll);

    		glm::vec3 right;
    		right.x = c1 * c3 + s1 * s2 * s3;
    		right.y = c2 * s3;
    		right.z = c1 * s2 * s3 - c3 * s1;

    		glm::vec3 up;
    		up.x = c3 * s1 * s2 - c1 * s3;
    		up.y = c2 * c3;
    		up.z = c1 * c3 * s2 + s1 * s3;

    		glm::vec3 forward;
    		forward.x = c2 * s1;
    		forward.y = -s2;
    		forward.z = c1 * c2;

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
    	m_camera->recalculateMatrices();
    }
} // Vectrix