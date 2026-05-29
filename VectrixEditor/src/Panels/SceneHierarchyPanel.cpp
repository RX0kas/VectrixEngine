#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "Vectrix/Scene/Component.h"
#include <glm/gtc/type_ptr.hpp>

namespace Vectrix {
    SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &scene) {
        setContext(scene);
    }

    void SceneHierarchyPanel::setContext(const std::shared_ptr<Scene> &scene) {
        m_context = scene;
    }

    void SceneHierarchyPanel::onImGuiRender() {
        ImGui::Begin("Scene Hierarchy");
        for(entt::entity entityID: m_context->m_registry.view<entt::entity>()) {
            Entity entity{ entityID , m_context.get() };
            drawEntityNode(entity);
        }
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_selectionContext = {};

        if (ImGui::BeginPopupContextWindow(0, 1)) {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_context->createEntity("Empty Entity");

            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Begin("Properties");
        if (m_selectionContext) {
            drawProperties(m_selectionContext);

            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent")) {
            bool hasOneComponent = false;
            if (!m_selectionContext.hasComponent<CameraComponent>()) {
                hasOneComponent = true;
                if (ImGui::MenuItem("Camera")) {
                    m_selectionContext.addComponent<CameraComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!m_selectionContext.hasComponent<MeshRenderer>()) {
                hasOneComponent = true;
                if (ImGui::MenuItem("MeshRenderer")) {
                    m_selectionContext.addComponent<MeshRenderer>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!hasOneComponent) {
                ImGui::Text("No component can be added");
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::drawEntityNode(Entity& entity) {
        const char* name = entity.getComponent<InformationComponent>().name.c_str();
        if (ImGui::Selectable(name)) {
            m_selectionContext = entity;
        }
    }

    void SceneHierarchyPanel::drawProperties(Entity entity) {
        // InformationComponent
        {
            auto& ic = entity.getComponent<InformationComponent>();
            char buffer[256] = {};
            strcpy(buffer,ic.name.c_str());
            if (ImGui::InputText("Name",buffer,sizeof(buffer)))
                ic.name = std::string(buffer);
        }

        // TransformComponent
        {
            auto& tc = entity.getComponent<TransformComponent>();
            bool mustBeRemoved = false;
            if (drawTreeNodeComponent("Transform",mustBeRemoved,false)) {
                // Position
                float p[3] = {tc.position.x,tc.position.y,tc.position.z};
                if (ImGui::DragFloat3("Position",p,0.1))
                    tc.position = {p[0],p[1],p[2]};

                // Rotation
                float r[3] = {tc.rotation.x,tc.rotation.y,tc.rotation.z};
                if (ImGui::DragFloat3("Rotation",r,0.1))
                    tc.rotation = {r[0],r[1],r[2]};

                // Scale
                float s[3] = {tc.scale.x,tc.scale.y,tc.scale.z};
                if (ImGui::DragFloat3("Scale",s,0.1))
                    tc.scale = {s[0],s[1],s[2]};

                ImGui::TreePop();
            }
        }
        // MeshComponent
        if (entity.hasComponent<MeshRenderer>()) {
            auto& mc = entity.getComponent<MeshRenderer>();
            bool mustBeRemoved = false;

            if (drawTreeNodeComponent("Mesh Renderer", mustBeRemoved)) {
                bool isEnable = mc.isEnable();

                if (ImGui::Checkbox("Enable", &isEnable)) {
                    if (isEnable) {
                        if (!mc.tryEnabling()) {
                            isEnable = false;
                            ImGui::OpenPopup("MeshRendererEnableError");
                        }
                    } else {
                        mc.disable();
                    }
                }

                if (ImGui::BeginPopup("MeshRendererEnableError")) { // TODO: make an error popup
                    ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Cannot enable Mesh Renderer");
                    ImGui::Separator();
                    ImGui::Text("Missing required data:");
                    if (mc.shader == nullptr)  ImGui::BulletText("Shader is not set");
                    if (mc.texture == nullptr) ImGui::BulletText("Texture is not set");
                    if (mc.vertexArray == nullptr) ImGui::BulletText("VertexArray is not set");
                    if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                ImGui::BeginDisabled(!isEnable);

                showField("Shader", mc.shader, mc.shader ? mc.shader->getName() : "");
                showField("Texture", mc.texture, mc.texture ? mc.texture->getName() : "");

                if (mc.vertexArray != nullptr)
                    ImGui::Text("VertexArray: %p", mc.vertexArray.get());
                else
                    ImGui::TextDisabled("VertexArray: nullptr");

                ImGui::EndDisabled();
                ImGui::TreePop();
            }

            if (mustBeRemoved)
                entity.deleteComponent<MeshRenderer>();
        }

        // CameraComponent
        if (entity.hasComponent<CameraComponent>()) {
            auto& cc = entity.getComponent<CameraComponent>();
            bool mustBeRemoved = false;
            if (drawTreeNodeComponent("Camera",mustBeRemoved)) {
                Camera& camera = cc.camera;
                bool changed = false;
                float fov = camera.getFOV();
                float camNear = camera.getCamNear();
                float camFar = camera.getCamFar();
                if (ImGui::DragFloat("FOV",&fov,0.01,0.01)) {
                    camera.setFOV(fov);
                    changed = true;
                }
                if (ImGui::DragFloat("camNear",&camNear,0.01,0)) {
                    camera.setCamNear(camNear);
                    changed = true;
                }
                if (ImGui::DragFloat("camFar",&camFar,1,0,INTMAX_MAX,"%.1f")) {
                    camera.setCamFar(camFar);
                    changed = true;
                }
                if (changed)
                    camera.recalculateMatrices();

                ImGui::TreePop();
            }

            if (mustBeRemoved) {
                entity.deleteComponent<CameraComponent>();
            }
        }
    }

    bool SceneHierarchyPanel::drawTreeNodeComponent(const std::string& text, bool& mustBeRemoved, bool removable) {
        ImGuiTreeNodeFlags_ flags = ImGuiTreeNodeFlags_Framed;

        const float buttonWidth = ImGui::CalcTextSize("···").x + ImGui::GetStyle().FramePadding.x * 2.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

        ImGui::SetNextItemAllowOverlap();
        bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(std::hash<std::string>{}(text)), flags, "%s",text.c_str());
        ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - buttonWidth);

        if (ImGui::Button(("···##" + text).c_str(), ImVec2{ buttonWidth, 0 })) {
            ImGui::OpenPopup(("ComponentSettings##" + text).c_str());
        }
        ImGui::PopStyleVar();

        // PopUp
        if (ImGui::BeginPopup(("ComponentSettings##" + text).c_str())) {
            if (ImGui::MenuItem("Wiki (WiP)")) {
                ImGui::CloseCurrentPopup();
            }
            if (removable) {
                if (ImGui::MenuItem("Delete Component")) {
                    mustBeRemoved = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
        return open;
    }
} // Vectrix