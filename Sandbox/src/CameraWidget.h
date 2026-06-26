#ifndef VECTRIXWORKSPACE_CAMERAWIDGET_H
#define VECTRIXWORKSPACE_CAMERAWIDGET_H
#include "Vectrix.h"


class CameraWidget : public Vectrix::ImGuiWidget {
public:
    CameraWidget(std::shared_ptr<Vectrix::Entity> camera);
    ~CameraWidget() override;
    void render() override;
private:
    std::shared_ptr<Vectrix::Entity> m_camera;
};

#endif //VECTRIXWORKSPACE_CAMERAWIDGET_H