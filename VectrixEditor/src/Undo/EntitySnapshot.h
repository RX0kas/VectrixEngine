#ifndef VECTRIXWORKSPACE_ENTITYSNAPSHOT_H
#define VECTRIXWORKSPACE_ENTITYSNAPSHOT_H

#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * @file EntitySnapshot.h
 * @brief Capture/restore helpers for the components a Command needs to undo across
 *        an entity or a component being destroyed and later recreated
 */

namespace Vectrix {
    class Entity;
    class Mesh;
    class Texture;
    class Shader;

    /// TransformComponent's fields, captured by value so a command can hold them across frames
    struct TransformSnapshot {
        glm::vec3 position{0.0f};
        glm::vec3 scale{1.0f};
        glm::quat rotation = glm::identity<glm::quat>();
    };

    /// MeshRendererComponent's fields — captured individually since the component itself is non-copyable
    struct MeshRendererSnapshot {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Texture> texture;
        std::shared_ptr<Shader> shader;
        bool enabled = false;
    };

    /// CameraComponent's fields — captured individually rather than copying Camera (it holds a
    /// TransformComponent& that would alias the live entity's transform if copy-constructed)
    struct CameraSnapshot {
        float fov = 50.0f;
        float camNear = 0.1f;
        float camFar = 1000.0f;
        bool hasCustomAspect = false;
        float customAspect = -1.0f;
        bool isCurrent = false;
    };

    /// Every component an entity can carry, snapshotted for delete/undo-delete
    struct EntitySnapshot {
        std::string name;
        TransformSnapshot transform;

        bool hasMeshRenderer = false;
        MeshRendererSnapshot meshRenderer;

        bool hasCamera = false;
        CameraSnapshot camera;
    };

    /// Read `entity`'s current components into a snapshot
    EntitySnapshot captureEntitySnapshot(const std::shared_ptr<Entity>& entity);

    /// Re-add components from `snapshot` onto `entity` (freshly created: has only Information+Transform)
    void applyEntitySnapshot(const std::shared_ptr<Entity>& entity, const EntitySnapshot& snapshot);

    CameraSnapshot captureCameraSnapshot(const std::shared_ptr<Entity>& entity);
    void applyCameraSnapshot(const std::shared_ptr<Entity>& entity, const CameraSnapshot& snapshot);

    MeshRendererSnapshot captureMeshRendererSnapshot(const std::shared_ptr<Entity>& entity);
    void applyMeshRendererSnapshot(const std::shared_ptr<Entity>& entity, const MeshRendererSnapshot& snapshot);
} // Vectrix

#endif //VECTRIXWORKSPACE_ENTITYSNAPSHOT_H
