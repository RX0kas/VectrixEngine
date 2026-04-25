#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    Scene::Scene() {

    }

    Scene::~Scene() {

    }

    void Scene::OnUpdate(DeltaTime dt) {

    }

    void Scene::OnRender() {
        auto group = m_registry.group<TransformComponent>(entt::get<MeshComponent>);
        for (auto entity : group) {
            auto [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
            if (mesh.enable) {
                Renderer::submit(mesh.shader,mesh.vertexArray,transform.modelMatrix(),mesh.shader.useTexture(mesh.texture));
            }
        }
    }

    Entity Scene::createEntity(const std::string& name) {
        Entity entity = { m_registry.create(), this };
        auto& tag = entity.addComponent<InformationComponent>();
        tag.name = name.empty() ? "Entity" : name;
        return entity;
    }
} // Vectrix