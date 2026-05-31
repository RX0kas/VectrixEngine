#ifndef VECTRIXWORKSPACE_GIZMO_H
#define VECTRIXWORKSPACE_GIZMO_H

#include "imgui.h"
#include "ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"
#include "Vectrix/Input/Input.h"
#include "Vectrix/Input/KeyCodes.h"
#include "Vectrix/Scene/Entity.h"
#include "Vectrix/Scene/Components/CameraComponent.h"

namespace Vectrix {
    inline void useGizmo(Entity& selectedEntity, Entity& cameraEntity, int gizmoType,ImVec2 windowPos,ImVec2 windowSize) {
        if (selectedEntity && gizmoType != -1) {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(windowPos.x, windowPos.y+ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0f, windowSize.x, windowSize.y);

            // Camera
            const auto& camera = cameraEntity.getComponent<CameraComponent>().camera;
            const glm::mat4& cameraProjectionVulkan = camera.getProjectionMatrix();
            glm::mat4 cameraProjection = cameraProjectionVulkan;
            cameraProjection[1][1] *= -1;

            glm::mat4 cameraView = camera.getViewMatrix();

            // Entity transform
            auto& tc = selectedEntity.getComponent<TransformComponent>();
            glm::mat4 transform = tc.modelMatrix();

            // Snapping
            bool snap = Input::isKeyPressed(VC_KEY_LEFT_CONTROL);
            float snapValue = 0.5f; // Snap to 0.5m for translation/scale
            // Snap to 45 degrees for rotation
            if (gizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 45.0f;

            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                static_cast<ImGuizmo::OPERATION>(gizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing()) {
                glm::vec3 position, scale;
                glm::quat rotation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(transform, scale, rotation, position, skew, perspective);

                tc.position = position;
                tc.rotation = rotation;
                tc.scale = scale;
            }
        }
    }
}

#endif //VECTRIXWORKSPACE_GIZMO_H