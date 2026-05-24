#ifndef VECTRIXWORKSPACE_CAMERACOMPONENT_H
#define VECTRIXWORKSPACE_CAMERACOMPONENT_H
#include "Vectrix/Rendering/Camera/Camera.h"

namespace Vectrix {
    struct CameraComponent {
        Camera camera;

        CameraComponent(const CameraComponent&) = default;
        CameraComponent(TransformComponent& transform) : camera(transform) {}
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_CAMERACOMPONENT_H