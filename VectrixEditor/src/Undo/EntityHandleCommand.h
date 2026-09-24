#ifndef VECTRIXWORKSPACE_ENTITYHANDLECOMMAND_H
#define VECTRIXWORKSPACE_ENTITYHANDLECOMMAND_H

#include <memory>

#include "Command.h"

/**
 * @file EntityHandleCommand.h
 * @brief Marker interface for commands that create/destroy an entity
 */

namespace Vectrix {
    class Entity;

    /**
     * @brief A Command whose execute()/undo() replaces which entt entity it targets
     *
     * entt recycles entity ids, so a command that creates or destroys an entity gets a
     * different handle every time it runs (see Scene::createEntity). Callers that need to
     * keep something (e.g. the scene hierarchy's selection) pointed at "the same" logical
     * entity across an undo/redo cycle read currentHandle() right after calling
     * UndoHistory::push/undo/redo.
     */
    class EntityHandleCommand : public Command {
    public:
        /// The entity the command currently targets, or nullptr right after an undo that destroyed it
        [[nodiscard]] virtual std::shared_ptr<Entity> currentHandle() const = 0;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_ENTITYHANDLECOMMAND_H
