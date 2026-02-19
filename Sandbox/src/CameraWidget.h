#ifndef VECTRIXWORKSPACE_CAMERAWIDGET_H
#define VECTRIXWORKSPACE_CAMERAWIDGET_H
#include "Vectrix/ImGui/ImGuiWidget.h"
#include "Vectrix/Renderer/Camera/PerspectiveCamera.h"


class CameraWidget : public Vectrix::ImGuiWidget {
public:
    CameraWidget(Vectrix::PerspectiveCamera &camera);
    ~CameraWidget() override;
    void render() override;
private:
    Vectrix::PerspectiveCamera &m_camera;
};

#endif //VECTRIXWORKSPACE_CAMERAWIDGET_H