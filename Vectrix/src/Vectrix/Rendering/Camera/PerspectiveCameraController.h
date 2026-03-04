#ifndef VECTRIXWORKSPACE_PERSPECTIVECAMERACONTROLLER_H
#define VECTRIXWORKSPACE_PERSPECTIVECAMERACONTROLLER_H
#include "PerspectiveCamera.h"
#include "Vectrix/Core/DeltaTime.h"
#include "Vectrix/Events/Event.h"
#include "Vectrix/Events/MouseEvent.h"
#include "Vectrix/Events/WindowEvent.h"

namespace Vectrix {
    class PerspectiveCameraController {
    public:
        PerspectiveCameraController(float cameraTranslationSpeed=5.0f,float cameraRotationSpeed=5.0f);

        void onUpdate(DeltaTime dt);
        void onEvent(Event& e);

        PerspectiveCamera& getCamera() { return m_camera; }
        
        void setCameraMoveSpeed(float m_camera_move_speed) {
            m_cameraMoveSpeed = m_camera_move_speed;
        }

        void setCameraRotationSpeed(float m_camera_rotation_speed) {
            m_cameraRotationSpeed = m_camera_rotation_speed;
        }

    private:
        bool onWindowResized(WindowResizeEvent& e);

        PerspectiveCamera m_camera;
        float m_cameraMoveSpeed, m_cameraRotationSpeed;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_PERSPECTIVECAMERACONTROLLER_H