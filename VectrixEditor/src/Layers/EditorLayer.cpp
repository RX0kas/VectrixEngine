#include "EditorLayer.h"

namespace Vectrix {
    EditorLayer::EditorLayer() : Layer("VC_Editor"),m_viewportSize(1,1) {
        FramebufferSpecification fbSpec;
        fbSpec.width = 1;
        fbSpec.height = 1;
        m_framebuffer = Framebuffer::create(fbSpec);

    	ShaderUniformLayout layout;
    	m_viewportShader = ShaderManager::createShader("VC_viewport", "./shaders/viewport.vert", "./shaders/viewport.frag",layout);
    	m_testModel = MeshManager::loadModel("VC_testModel","./models/fox.obj");
    	m_testTexture = TextureManager::createTexture("VC_testTexture", "./textures/fox.png");
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

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen)
			{
				ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}

			// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
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
        m_framebuffer->bind();
        Renderer::beginScene(m_cameraController.getCamera());

    	m_viewportShader->setTexture(0,m_testTexture);
    	Renderer::submit(*m_viewportShader,*m_testModel);

        Renderer::endScene();
        m_framebuffer->unbind();
    }

    void EditorLayer::OnUpdate(const DeltaTime &dt) {
        m_cameraController.onUpdate(dt);
    	if (m_mustResize) {
    		m_framebuffer->resize(m_viewportSize);
    		m_cameraController.getCamera().setCustomAspect(m_viewportSize.x/m_viewportSize.y);
    		m_mustResize = false;
    	}
    }
} // Vectrix