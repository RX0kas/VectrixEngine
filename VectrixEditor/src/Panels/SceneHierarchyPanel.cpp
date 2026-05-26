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

        ImGui::End();

        ImGui::Begin("Properties");
        if (m_selectionContext)
            drawProperties(m_selectionContext);

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
            if (ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(TransformComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
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
        if (entity.hasComponent<MeshComponent>()) {
            auto& mc = entity.getComponent<MeshComponent>();
            if (ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(MeshComponent).hash_code()), ImGuiTreeNodeFlags_None, "Mesh")) {
                ImGui::Checkbox("Enable",&mc.enable);
                ImGui::BeginDisabled(!mc.enable);
                ImGui::Text("Shader: %s",mc.shader->getName().c_str());
                ImGui::Text("Texture: %s",mc.texture->getName().c_str());
                ImGui::Text("VertexArray: %p",&mc.vertexArray);
                ImGui::Text("BufferLayout: %p",&mc.layout);
                ImGui::EndDisabled();
                ImGui::TreePop();
            }
        }

        // CameraComponent
        if (entity.hasComponent<CameraComponent>()) {
            auto& cc = entity.getComponent<CameraComponent>();
            if (ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(CameraComponent).hash_code()), ImGuiTreeNodeFlags_None, "Camera")) {
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
        }
    }
} // Vectrix