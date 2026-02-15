#include "CameraWidget.h"

#include "imgui.h"
#include "Vectrix/Utils/Math.h"

CameraWidget::CameraWidget(Vectrix::PerspectiveCamera &camera) : m_camera(camera) {

}

CameraWidget::~CameraWidget() = default;

void CameraWidget::render() {
    ImGui::Begin("Debug Camera");
    float pos[3] = {m_camera.getPosition().x,m_camera.getPosition().y,m_camera.getPosition().z};
    if (ImGui::SliderFloat3("Position",pos,-VC_2PI,VC_2PI)) {
        m_camera.setPosition({pos[0],pos[1],pos[2]});
    }
    float rot[3] = {m_camera.getRotation().x,m_camera.getRotation().y,m_camera.getRotation().z};
    if (ImGui::SliderFloat3("Rotation",rot,-VC_2PI,VC_2PI)) {
        m_camera.setRotation({rot[0],rot[1],rot[2]});
    }
    ImGui::End();
}
