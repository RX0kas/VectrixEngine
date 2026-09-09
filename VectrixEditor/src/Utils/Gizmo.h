#ifndef VECTRIXWORKSPACE_GIZMO_H
#define VECTRIXWORKSPACE_GIZMO_H

#include "imgui.h"
#include "ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"
#include "Vectrix/Input/Input.h"
#include "Vectrix/Input/KeyCodes.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
    inline void useGizmo(const std::shared_ptr<Entity>& selectedEntity, const EditorCamera& camera, int gizmoType,ImVec2 windowPos,ImVec2 windowSize,
                         float translationSnap = 0.5f, float rotationSnap = 45.0f) {
        if (selectedEntity && gizmoType != -1) {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

            // Camera
            const glm::mat4& cameraProjectionVulkan = camera.getProjectionMatrix();
            glm::mat4 cameraProjection = cameraProjectionVulkan;
            cameraProjection[1][1] *= -1;

            glm::mat4 cameraView = camera.getViewMatrix();

            // Entity transform
            auto& tc = selectedEntity->getComponent<TransformComponent>();
            glm::mat4 transform = tc.modelMatrix();

            // Snapping (hold Ctrl); snap distances come from the editor settings.
            bool snap = Input::isKeyPressed(VC_KEY_LEFT_CONTROL);
            float snapValue = (gizmoType == ImGuizmo::OPERATION::ROTATE) ? rotationSnap : translationSnap;

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