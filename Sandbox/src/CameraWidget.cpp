#include "CameraWidget.h"

#include "imgui.h"

CameraWidget::CameraWidget(Vectrix::Entity &camera) : m_camera(camera) {

}

CameraWidget::~CameraWidget() = default;

void CameraWidget::render() {
    ImGui::Begin("Debug Camera",nullptr,ImGuiWindowFlags_NoNavInputs);
    glm::vec3 position = m_camera.getComponent<Vectrix::TransformComponent>().position;

    float pos[3] = {position.x,position.y,position.z};
    if (ImGui::SliderFloat3("Position",pos,-VC_2PI,VC_2PI)) {
        m_camera.getComponent<Vectrix::TransformComponent>().position = {pos[0],pos[1],pos[2]};
    }

    glm::vec3 rotation = m_camera.getComponent<Vectrix::TransformComponent>().getRotationDeg();
    float rot[3] = {rotation.x,rotation.y,rotation.z};
    if (ImGui::SliderFloat3("Rotation",rot,-VC_2PI,VC_2PI)) {
        m_camera.getComponent<Vectrix::TransformComponent>().setRotationDeg({rot[0],rot[1],rot[2]});
    }
    ImGui::End();
}
