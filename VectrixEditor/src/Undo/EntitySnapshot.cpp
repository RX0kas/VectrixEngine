#include "EntitySnapshot.h"

#include "Vectrix/Scene/Component.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
    CameraSnapshot captureCameraSnapshot(const std::shared_ptr<Entity>& entity) {
        CameraSnapshot snapshot;
        const Camera& camera = entity->getComponent<CameraComponent>().camera;
        snapshot.fov = camera.getFOV();
        snapshot.camNear = camera.getCamNear();
        snapshot.camFar = camera.getCamFar();
        snapshot.hasCustomAspect = camera.hasCustomAspect();
        snapshot.customAspect = camera.getAspect();
        snapshot.isCurrent = camera.isCurrent();
        return snapshot;
    }

    void applyCameraSnapshot(const std::shared_ptr<Entity>& entity, const CameraSnapshot& snapshot) {
        Camera& camera = entity->hasComponent<CameraComponent>()
            ? entity->getComponent<CameraComponent>().camera
            : entity->addComponent<CameraComponent>().camera;

        camera.setFOV(snapshot.fov);
        camera.setCamNear(snapshot.camNear);
        camera.setCamFar(snapshot.camFar);
        if (snapshot.hasCustomAspect)
            camera.setCustomAspect(snapshot.customAspect);
        if (snapshot.isCurrent)
            camera.setAsCurrent();
    }

    MeshRendererSnapshot captureMeshRendererSnapshot(const std::shared_ptr<Entity>& entity) {
        const MeshRendererComponent& mc = entity->getComponent<MeshRendererComponent>();
        return { mc.mesh, mc.texture, mc.shader, mc.isEnable() };
    }

    void applyMeshRendererSnapshot(const std::shared_ptr<Entity>& entity, const MeshRendererSnapshot& snapshot) {
        MeshRendererComponent& mc = entity->hasComponent<MeshRendererComponent>()
            ? entity->getComponent<MeshRendererComponent>()
            : entity->addComponent<MeshRendererComponent>();

        mc.mesh = snapshot.mesh;
        mc.texture = snapshot.texture;
        mc.shader = snapshot.shader;
        if (snapshot.enabled)
            mc.tryEnabling();
        else
            mc.disable();
    }

    EntitySnapshot captureEntitySnapshot(const std::shared_ptr<Entity>& entity) {
        EntitySnapshot snapshot;
        snapshot.name = entity->getComponent<InformationComponent>().name;

        const TransformComponent& tc = entity->getComponent<TransformComponent>();
        snapshot.transform = { tc.position, tc.scale, tc.rotation };

        snapshot.hasMeshRenderer = entity->hasComponent<MeshRendererComponent>();
        if (snapshot.hasMeshRenderer)
            snapshot.meshRenderer = captureMeshRendererSnapshot(entity);

        snapshot.hasCamera = entity->hasComponent<CameraComponent>();
        if (snapshot.hasCamera)
            snapshot.camera = captureCameraSnapshot(entity);

        return snapshot;
    }

    void applyEntitySnapshot(const std::shared_ptr<Entity>& entity, const EntitySnapshot& snapshot) {
        entity->getComponent<InformationComponent>().name = snapshot.name;

        TransformComponent& tc = entity->getComponent<TransformComponent>();
        tc.position = snapshot.transform.position;
        tc.scale = snapshot.transform.scale;
        tc.rotation = snapshot.transform.rotation;

        if (snapshot.hasMeshRenderer)
            applyMeshRendererSnapshot(entity, snapshot.meshRenderer);

        if (snapshot.hasCamera)
            applyCameraSnapshot(entity, snapshot.camera);
    }
} // Vectrix
