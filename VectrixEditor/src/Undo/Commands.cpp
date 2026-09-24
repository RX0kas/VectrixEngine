#include "Commands.h"

#include "Vectrix/Scene/Component.h"
#include "Vectrix/Scene/Entity.h"
#include "Vectrix/Scene/Scene.h"

namespace Vectrix {
    // --- TransformChangeCommand ---

    TransformChangeCommand::TransformChangeCommand(std::shared_ptr<Entity> entity, TransformSnapshot before, TransformSnapshot after)
        : m_entity(std::move(entity)), m_before(before), m_after(after) {}

    void TransformChangeCommand::execute() {
        TransformComponent& tc = m_entity->getComponent<TransformComponent>();
        tc.position = m_after.position;
        tc.scale = m_after.scale;
        tc.rotation = m_after.rotation;
    }

    void TransformChangeCommand::undo() {
        TransformComponent& tc = m_entity->getComponent<TransformComponent>();
        tc.position = m_before.position;
        tc.scale = m_before.scale;
        tc.rotation = m_before.rotation;
    }

    // --- RemoveCameraComponentCommand ---

    RemoveCameraComponentCommand::RemoveCameraComponentCommand(std::shared_ptr<Entity> entity)
        : m_entity(std::move(entity)), m_snapshot(captureCameraSnapshot(m_entity)) {}

    void RemoveCameraComponentCommand::execute() {
        m_entity->deleteComponent<CameraComponent>();
    }

    void RemoveCameraComponentCommand::undo() {
        applyCameraSnapshot(m_entity, m_snapshot);
    }

    // --- RemoveMeshRendererComponentCommand ---

    RemoveMeshRendererComponentCommand::RemoveMeshRendererComponentCommand(std::shared_ptr<Entity> entity)
        : m_entity(std::move(entity)), m_snapshot(captureMeshRendererSnapshot(m_entity)) {}

    void RemoveMeshRendererComponentCommand::execute() {
        m_entity->deleteComponent<MeshRendererComponent>();
    }

    void RemoveMeshRendererComponentCommand::undo() {
        applyMeshRendererSnapshot(m_entity, m_snapshot);
    }

    // --- SetCurrentCameraCommand ---

    SetCurrentCameraCommand::SetCurrentCameraCommand(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> previousCurrent)
        : m_entity(std::move(entity)), m_previousCurrent(std::move(previousCurrent)) {}

    void SetCurrentCameraCommand::execute() {
        m_entity->getComponent<CameraComponent>().camera.setAsCurrent();
    }

    void SetCurrentCameraCommand::undo() {
        if (m_previousCurrent)
            m_previousCurrent->getComponent<CameraComponent>().camera.setAsCurrent();
        else
            Camera::clearCurrent();
    }

    // --- CreateEntityCommand ---

    CreateEntityCommand::CreateEntityCommand(std::shared_ptr<Scene> scene, std::string name)
        : m_scene(std::move(scene)), m_name(std::move(name)) {}

    void CreateEntityCommand::execute() {
        m_entity = m_scene->createEntity(m_name);
    }

    void CreateEntityCommand::undo() {
        m_scene->destroyEntity(*m_entity);
        m_entity = nullptr;
    }

    // --- DeleteEntityCommand ---

    DeleteEntityCommand::DeleteEntityCommand(std::shared_ptr<Scene> scene, const std::shared_ptr<Entity>& entity)
        : m_scene(std::move(scene)), m_entity(entity), m_snapshot(captureEntitySnapshot(entity)) {}

    void DeleteEntityCommand::execute() {
        m_scene->destroyEntity(*m_entity);
        m_entity = nullptr;
    }

    void DeleteEntityCommand::undo() {
        m_entity = m_scene->createEntity(m_snapshot.name);
        applyEntitySnapshot(m_entity, m_snapshot);
    }
} // Vectrix
