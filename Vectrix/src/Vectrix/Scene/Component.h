#ifndef VECTRIXWORKSPACE_COMPONENT_H
#define VECTRIXWORKSPACE_COMPONENT_H
#include <memory>
#include <string>
#include <utility>

namespace Vectrix {
    class Entity;

    /**
     * @brief This component is a mandatory component of all entity
     */
    struct InformationComponent {
        std::string name;

        InformationComponent(std::shared_ptr<Entity> entity) {}
        InformationComponent(const InformationComponent&) = default;
        InformationComponent(std::shared_ptr<Entity> entity, std::string name) : name(std::move(name)) {}
    };
}

#include "Components/TransformComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/CameraComponent.h"

#endif //VECTRIXWORKSPACE_COMPONENT_H