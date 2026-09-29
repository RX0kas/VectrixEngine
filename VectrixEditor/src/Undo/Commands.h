#ifndef VECTRIXWORKSPACE_COMMANDS_H
#define VECTRIXWORKSPACE_COMMANDS_H

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "Command.h"
#include "EntityHandleCommand.h"
#include "EntitySnapshot.h"
#include "Vectrix/Scene/Entity.h"

/**
 * @file Commands.h
 * @brief Concrete Command types used by the editor's undo/redo system
 */

namespace Vectrix {
    class Scene;
    class Entity;

    /**
     * @brief Generic undo/redo of a single value, applied through a setter closure
     * @tparam T The value's type
     * @note The setter should re-fetch the component it writes to (e.g. via
     *       entity->getComponent<T>()) rather than capturing a raw pointer/reference to
     *       it: EnTT's storage for a component type can move when other entities gain or
     *       lose that same component type.
     */
    template <typename T>
    class PropertyChangeCommand : public Command {
    public:
        using Setter = std::function<void(const T&)>;

        PropertyChangeCommand(std::string label, T before, T after, Setter setter)
            : m_label(std::move(label)), m_before(std::move(before)),
              m_after(std::move(after)), m_setter(std::move(setter)) {}

        void execute() override { m_setter(m_after); }
        void undo() override { m_setter(m_before); }
        [[nodiscard]] const char* name() const override { return m_label.c_str(); }

    private:
        std::string m_label;
        T m_before;
        T m_after;
        Setter m_setter;
    };

    /**
     * @brief Undo/redo of a TransformComponent edit (position+scale+rotation as one step)
     *
     * Used both for a single inspector drag (one field changes) and for a gizmo drag
     * (several fields can change together) — either way, one user gesture is one command.
     */
    class TransformChangeCommand : public Command {
    public:
        TransformChangeCommand(std::shared_ptr<Entity> entity, TransformSnapshot before, TransformSnapshot after);
        void execute() override;
        void undo() override;
        [[nodiscard]] const char* name() const override { return "Transform"; }

    private:
        std::shared_ptr<Entity> m_entity;
        TransformSnapshot m_before;
        TransformSnapshot m_after;
    };

    /**
     * @brief Undo/redo of adding a component that didn't previously exist
     * @tparam T The component type; must be default-constructible via Entity::addComponent<T>()
     * @note Only for components that need no field restoration when re-added (CameraComponent
     *       and MeshRendererComponent both start empty/default) — removing a component that
     *       already had data uses a dedicated command instead (see RemoveCameraComponentCommand,
     *       RemoveMeshRendererComponentCommand) so that data survives an undo.
     */
    template <typename T>
    class AddComponentCommand : public Command {
    public:
        explicit AddComponentCommand(std::shared_ptr<Entity> entity) : m_entity(std::move(entity)) {}
        void execute() override { m_entity->template addComponent<T>(); }
        void undo() override { m_entity->template deleteComponent<T>(); }
        [[nodiscard]] const char* name() const override { return "Add Component"; }

    private:
        std::shared_ptr<Entity> m_entity;
    };

    /// Undo/redo of removing a CameraComponent, restoring its fields on undo
    class RemoveCameraComponentCommand : public Command {
    public:
        explicit RemoveCameraComponentCommand(std::shared_ptr<Entity> entity);
        void execute() override;
        void undo() override;
        [[nodiscard]] const char* name() const override { return "Remove Camera"; }

    private:
        std::shared_ptr<Entity> m_entity;
        CameraSnapshot m_snapshot; // captured at construction, before the first execute() removes it
    };

    /// Undo/redo of removing a MeshRendererComponent, restoring its fields on undo
    class RemoveMeshRendererComponentCommand : public Command {
    public:
        explicit RemoveMeshRendererComponentCommand(std::shared_ptr<Entity> entity);
        void execute() override;
        void undo() override;
        [[nodiscard]] const char* name() const override { return "Remove Mesh Renderer"; }

    private:
        std::shared_ptr<Entity> m_entity;
        MeshRendererSnapshot m_snapshot;
    };

    /// Undo/redo of the "Set as current" camera button
    class SetCurrentCameraCommand : public Command {
    public:
        SetCurrentCameraCommand(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> previousCurrent);
        void execute() override;
        void undo() override;
        [[nodiscard]] const char* name() const override { return "Set Current Camera"; }

    private:
        std::shared_ptr<Entity> m_entity;
        std::shared_ptr<Entity> m_previousCurrent; // nullptr when no camera was current before
    };

    /**
     * @brief Undo/redo of creating a new empty entity
     * @note entt recycles entity ids, so a fresh handle is produced every execute() (first
     *       run AND every redo) — currentHandle() always reflects the live one, use it to
     *       keep e.g. the scene hierarchy's selection pointed at the right entity.
     */
    class CreateEntityCommand : public EntityHandleCommand {
    public:
        CreateEntityCommand(std::shared_ptr<Scene> scene, std::string name);
        void execute() override;
        void undo() override;
        [[nodiscard]] const char* name() const override { return "Create Entity"; }
        [[nodiscard]] std::shared_ptr<Entity> currentHandle() const override { return m_entity; }

    private:
        std::shared_ptr<Scene> m_scene;
        std::string m_name;
        std::shared_ptr<Entity> m_entity;
    };

    /**
     * @brief Undo/redo of deleting an entity, restoring every component it had on undo
     * @note The snapshot is captured once, in the constructor, before the entity is ever
     *       destroyed — not re-captured on a later execute(), which would otherwise try to
     *       read components off an entity that no longer exists.
     */
    class DeleteEntityCommand : public EntityHandleCommand {
    public:
        DeleteEntityCommand(std::shared_ptr<Scene> scene, const std::shared_ptr<Entity>& entity);
        void execute() override;
        void undo() override;
        [[nodiscard]] const char* name() const override { return "Delete Entity"; }
        [[nodiscard]] std::shared_ptr<Entity> currentHandle() const override { return m_entity; }

    private:
        std::shared_ptr<Scene> m_scene;
        std::shared_ptr<Entity> m_entity;
        EntitySnapshot m_snapshot;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_COMMANDS_H
