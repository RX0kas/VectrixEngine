#ifndef VECTRIXWORKSPACE_COMPONENT_H
#define VECTRIXWORKSPACE_COMPONENT_H
#include <string>
#include <utility>

#include "glm/glm.hpp"

/**
 * @brief This component is a mandatory component of all entity
 */
struct InformationComponent {
    std::string name;

    InformationComponent() = default;
    InformationComponent(const InformationComponent&) = default;
    InformationComponent(std::string name) : name(std::move(name)) {}
};

#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"

#endif //VECTRIXWORKSPACE_COMPONENT_H