#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::OnUpdate(DeltaTime dt) {

    }

    void Scene::OnRender() {
        auto view = m_registry.view<MeshComponent, TransformComponent>();

        for (auto entity : view) {
            auto& mesh = view.get<MeshComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);

            if (!mesh.enable) continue;

            Renderer::submit(mesh.shader,mesh.vertexArray,transform.modelMatrix(),mesh.shader->useTexture(mesh.texture));
        }
    }

    Entity Scene::createEntity(const std::string& name) {
        Entity entity = { m_registry.create(), this };
        auto& tag = entity.addComponent<InformationComponent>();
        tag.name = name.empty() ? "Entity" : name;
        entity.addComponent<TransformComponent>();
        return entity;
    }
} // Vectrix