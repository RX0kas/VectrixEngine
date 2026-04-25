#include "EditorLayer.h"

namespace Vectrix {
    EditorLayer::EditorLayer() : Layer("VC_Editor"), m_viewportSize(1, 1), m_foxEntity(Entity::nullEntity()) {
	    FramebufferSpecification fbSpec;
	    fbSpec.width = 1;
	    fbSpec.height = 1;
	    m_framebuffer = Framebuffer::create(fbSpec);

	    m_activeScene = std::make_shared<Scene>();

	    ShaderUniformLayout layout;
	    m_viewportShader = ShaderManager::createShader("VC_viewport", "./shaders/viewport.vert","./shaders/viewport.frag", layout);
	    m_testTexture = TextureManager::createTexture("VC_testTexture", "./textures/fox.png");

	    m_foxEntity = m_activeScene->createEntity();
    	m_foxEntity.addComponent<TransformComponent>();
    	m_foxEntity.addComponent<MeshComponent>("./models/fox.obj",*m_viewportShader,m_testTexture);
    }

    void EditorLayer::OnEvent(Event &event) {
		m_cameraController.onEvent(event);
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
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}
    }

    void EditorLayer::OnRender() {

    }

    void EditorLayer::OnRenderOffscreen() {
    	m_cameraController.setCameraAsActive();
        m_framebuffer->bind();
    	Application::resetCache();
        m_activeScene->OnRender();
    	Application::flush();
        m_framebuffer->unbind();
    }

    void EditorLayer::OnUpdate(const DeltaTime &dt) {
    	m_cameraController.setCameraAsActive();
    	if (m_viewportFocused || m_viewportHovered)
    		m_cameraController.onUpdate(dt);
    	if (m_mustResize) {
    		m_framebuffer->resize(m_viewportSize);
    		m_cameraController.getCamera().setCustomAspect(m_viewportSize.x/m_viewportSize.y);
    		m_mustResize = false;
    	}
    	m_activeScene->OnUpdate(dt);
    }
} // Vectrix