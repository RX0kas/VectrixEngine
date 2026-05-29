#ifndef VECTRIXWORKSPACE_CAMERACOMPONENT_H
#define VECTRIXWORKSPACE_CAMERACOMPONENT_H
#include "Vectrix/Rendering/Camera/Camera.h"

namespace Vectrix {
    struct CameraComponent {
        Camera camera;

        CameraComponent(const CameraComponent&) = default;
        CameraComponent(Entity* entity) : camera(entity) {}
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_CAMERACOMPONENT_H