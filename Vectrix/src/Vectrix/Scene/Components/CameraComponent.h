#ifndef VECTRIXWORKSPACE_CAMERACOMPONENT_H
#define VECTRIXWORKSPACE_CAMERACOMPONENT_H
#include <memory>

#include "Vectrix/Rendering/Camera/Camera.h"


namespace Vectrix {
    struct CameraComponent {
        Camera camera;

        CameraComponent(const CameraComponent&) = default;
        CameraComponent(std::shared_ptr<Entity> entity) : camera(entity) {}
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_CAMERACOMPONENT_H