#ifndef VECTRIXWORKSPACE_PERSPECTIVECAMERACONTROLLER_H
#define VECTRIXWORKSPACE_PERSPECTIVECAMERACONTROLLER_H
#include "PerspectiveCamera.h"
#include "Vectrix/Core/DeltaTime.h"
#include "Vectrix/Events/Event.h"
#include "Vectrix/Events/MouseEvent.h"
#include "Vectrix/Events/WindowEvent.h"

/**
 * @file PerspectiveCameraController.h
 * @brief Definition of the default camera controller
 * @ingroup utils
 */

namespace Vectrix {
    /**
     * @brief This class is a basic camera controller
     */
    class PerspectiveCameraController {
    public:
        /**
         * @brief This creates a new Camera controller with a perspective projection
         * @param cameraTranslationSpeed The default camera move speed
         * @param cameraRotationSpeed The default camera rotation speed
         */
        PerspectiveCameraController(float cameraTranslationSpeed=5.0f,float cameraRotationSpeed=5.0f);
        /// @cond INTERNAL
        void onUpdate(DeltaTime dt);
        void onEvent(Event& e);
        /// @endcond

        /**
         * @brief This function return the camera object
         */
        PerspectiveCamera& getCamera() { return m_camera; }

        /**
         * @brief This function change the camera speed
         * @param camera_move_speed The new camera move speed
         */
        void setCameraMoveSpeed(float camera_move_speed) {
            m_cameraMoveSpeed = camera_move_speed;
        }

        /**
         * @brief This function change the camera rotation speed
         * @param camera_rotation_speed The new camera rotation speed
         */
        void setCameraRotationSpeed(float camera_rotation_speed) {
            m_cameraRotationSpeed = camera_rotation_speed;
        }

        /**
         * @brief This function set the camera as the current active camera
         */
        void setCameraAsActive() {
            m_camera.setAsCurrent();
        }
    private:
        bool onWindowResized(WindowResizeEvent& e);

        PerspectiveCamera m_camera;
        float m_cameraMoveSpeed, m_cameraRotationSpeed;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_PERSPECTIVECAMERACONTROLLER_H