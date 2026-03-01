#include "PerspectiveCameraController.h"

#include "Vectrix/Input/Input.h"
#include "Vectrix/Input/KeyCodes.h"

namespace Vectrix {
    PerspectiveCameraController::PerspectiveCameraController(float cameraTranslationSpeed,float cameraRotationSpeed) : m_cameraRotationSpeed(cameraRotationSpeed),m_cameraMoveSpeed(cameraTranslationSpeed) {

    }

    void PerspectiveCameraController::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(VC_BIND_EVENT_FN_RETURN(onWindowResized));
    }

    void PerspectiveCameraController::onUpdate(DeltaTime dt) {
        glm::vec3 m_cameraPosition = m_camera.getPosition();
        glm::vec3 m_cameraRotation = m_camera.getRotation();
        if (Input::isKeyPressed(VC_KEY_A))
            m_cameraPosition.x -= m_cameraMoveSpeed * dt;
        else if (Input::isKeyPressed(VC_KEY_D))
            m_cameraPosition.x += m_cameraMoveSpeed * dt;

        if (Input::isKeyPressed(VC_KEY_W))
            m_cameraPosition.z += m_cameraMoveSpeed * dt;
        else if (Input::isKeyPressed(VC_KEY_S))
            m_cameraPosition.z -= m_cameraMoveSpeed * dt;
        if (Input::isKeyPressed(VC_KEY_Q))
            m_cameraPosition.y += m_cameraMoveSpeed * dt;
        else if (Input::isKeyPressed(VC_KEY_E))
            m_cameraPosition.y -= m_cameraMoveSpeed * dt;

        if (Input::isKeyPressed(VC_KEY_LEFT))
            m_cameraRotation.y -= m_cameraRotationSpeed * dt;
        else if (Input::isKeyPressed(VC_KEY_RIGHT))
            m_cameraRotation.y += m_cameraRotationSpeed * dt;

        if (Input::isKeyPressed(VC_KEY_UP))
            m_cameraRotation.x += m_cameraRotationSpeed * dt;
        else if (Input::isKeyPressed(VC_KEY_DOWN))
            m_cameraRotation.x -= m_cameraRotationSpeed * dt;

        m_camera.setPosition(m_cameraPosition);
        m_camera.setRotation(m_cameraRotation);
    }

    bool PerspectiveCameraController::onWindowResized(WindowResizeEvent &e) {
        m_camera.recalculateMatrices();
        return false;
    }
} // Vectrix