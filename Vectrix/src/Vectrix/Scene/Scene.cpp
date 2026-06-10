#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "Vectrix/Rendering/Renderer.h"

namespace Vectrix {
    //Scene::Scene(const SceneCreationData& creationData) {
    //    if (!creationData.loaded) {
    //        VC_CORE_ERROR("The SceneCreationData has not been fully completed");
    //    }
//
    //    for (const auto& entityData : creationData.entities) {
    //        Entity entity = { m_registry.create(), this };
    //        auto& tag = entity.addComponent<InformationComponent>();
    //        tag.name = entityData.name;
    //        for (const auto& componentData : entityData.components) {
    //            if (entt::type_hash<TransformComponent>::value()==componentData.type_id) {
    //                TransformComponent t(&entity);
    //                std::memcpy(&t, componentData.data.data(), sizeof(TransformComponent));
    //                m_registry.emplace<TransformComponent>(entity.getID(), t);
    //            } else if (entt::type_hash<MeshRendererComponent>::value() == componentData.type_id) {
    //                MeshRendererComponent m(&entity);
    //                //std::memcpy(&m, componentData.data.data(), sizeof(MeshRendererComponent));
    //                m_registry.emplace<MeshRendererComponent>(entity.getID(), m);
    //            } else if (entt::type_hash<CameraComponent>::value()==componentData.type_id) {
    //                CameraComponent c(&entity);
    //                std::memcpy(&c, componentData.data.data(), sizeof(CameraComponent));
    //                m_registry.emplace<CameraComponent>(entity.getID(), c);
    //            }
    //        }
    //    }
    //}

    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::OnUpdate(DeltaTime dt) {

    }

    void Scene::OnRender() {
        auto view = m_registry.view<MeshRendererComponent, TransformComponent>();

        for (auto entity : view) {
            auto& mesh = view.get<MeshRendererComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);

            if (!mesh.isEnable()) continue;

            Renderer::submit(mesh.shader,mesh.mesh->getVertexArray(),transform.modelMatrix(),mesh.shader->useTexture(mesh.texture));
        }
    }

    Entity Scene::createEntity(const std::string& name) {
        Entity entity = { m_registry.create(), this };
        auto& tag = entity.addComponent<InformationComponent>();
        tag.name = name.empty() ? "Entity" : name;
        entity.addComponent<TransformComponent>();
        return entity;
    }

    void Scene::destroyEntity(Entity entity) {
        m_registry.destroy(entity);
    }

} // Vectrix