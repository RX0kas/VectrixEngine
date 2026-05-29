#ifndef VECTRIXWORKSPACE_COMPONENT_H
#define VECTRIXWORKSPACE_COMPONENT_H
#include <string>
#include <utility>

namespace Vectrix {
    /**
     * @brief This component is a mandatory component of all entity
     */
    struct InformationComponent {
        std::string name;

        InformationComponent(Entity* entity) {}
        InformationComponent(const InformationComponent&) = default;
        InformationComponent(Entity* entity, std::string name) : name(std::move(name)) {}
    };


}

#include "Components/TransformComponent.h"
#include "Components/MeshRenderer.h"
#include "Components/CameraComponent.h"

#endif //VECTRIXWORKSPACE_COMPONENT_H