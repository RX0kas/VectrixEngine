#include "PerspectiveCameraController.h"

#include "Vectrix/Debug/Profiler.h"
#include "Vectrix/Input/Input.h"
#include "Vectrix/Input/KeyCodes.h"

namespace Vectrix {
    PerspectiveCameraController::PerspectiveCameraController(float cameraTranslationSpeed,float cameraRotationSpeed) : m_cameraRotationSpeed(cameraRotationSpeed),m_cameraMoveSpeed(cameraTranslationSpeed) {

    }

    void PerspectiveCameraController::onEvent(Event &e) {
        VC_PROFILER_FUNCTION();
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(VC_BIND_EVENT_FN_RETURN(onWindowResized));
    }

    void PerspectiveCameraController::onUpdate(DeltaTime dt) {
        VC_PROFILER_FUNCTION();
        glm::vec3 cameraRot = m_camera.getRotation();
        if (Input::isKeyPressed(VC_KEY_LEFT))
            cameraRot.y -= m_cameraRotationSpeed * dt;
        if (Input::isKeyPressed(VC_KEY_RIGHT))
            cameraRot.y += m_cameraRotationSpeed * dt;
        if (Input::isKeyPressed(VC_KEY_UP))
            cameraRot.x += m_cameraRotationSpeed * dt;
        if (Input::isKeyPressed(VC_KEY_DOWN))
            cameraRot.x -= m_cameraRotationSpeed * dt;
        m_camera.setRotation(cameraRot);

        float yaw = cameraRot.y;
        float pitch = cameraRot.x;
        float roll = cameraRot.z;

        float c1 = std::cos(yaw);
        float s1 = std::sin(yaw);
        float c2 = std::cos(pitch);
        float s2 = std::sin(pitch);
        float c3 = std::cos(roll);
        float s3 = std::sin(roll);

        glm::vec3 right;
        right.x = c1 * c3 + s1 * s2 * s3;
        right.y = c2 * s3;
        right.z = c1 * s2 * s3 - c3 * s1;

        glm::vec3 up;
        up.x = c3 * s1 * s2 - c1 * s3;
        up.y = c2 * c3;
        up.z = c1 * c3 * s2 + s1 * s3;

        glm::vec3 forward;
        forward.x = c2 * s1;
        forward.y = -s2;
        forward.z = c1 * c2;

        glm::vec3 moveDir(0.0f);
        if (Input::isKeyPressed(VC_KEY_A)) moveDir.x -= 1.0f;
        if (Input::isKeyPressed(VC_KEY_D)) moveDir.x += 1.0f;
        if (Input::isKeyPressed(VC_KEY_W)) moveDir.z += 1.0f;
        if (Input::isKeyPressed(VC_KEY_S)) moveDir.z -= 1.0f;
        if (Input::isKeyPressed(VC_KEY_Q)) moveDir.y -= 1.0f;
        if (Input::isKeyPressed(VC_KEY_E)) moveDir.y += 1.0f;

        if (glm::length(moveDir) > 0.0f) {
            moveDir = glm::normalize(moveDir);

            glm::vec3 delta = (right * moveDir.x + forward * moveDir.z + up * moveDir.y) * m_cameraMoveSpeed * dt.getSeconds();

            m_camera.setPosition(m_camera.getPosition() + delta);
        }
    }

    bool PerspectiveCameraController::onWindowResized(WindowResizeEvent &e) {
        m_camera.recalculateMatrices();
        return false;
    }
} // Vectrix