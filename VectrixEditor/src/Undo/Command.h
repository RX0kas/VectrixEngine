#ifndef VECTRIXWORKSPACE_COMMAND_H
#define VECTRIXWORKSPACE_COMMAND_H

/**
 * @file Command.h
 * @brief The Command interface every undoable editor action implements
 */

namespace Vectrix {
    /**
     * @brief One undoable/redoable editor action
     * @see UndoHistory
     */
    class Command {
    public:
        virtual ~Command() = default;

        /**
         * @brief Apply the action
         * @note Also used to redo: it must be safe to call again after undo()
         */
        virtual void execute() = 0;

        /**
         * @brief Revert the action, back to the state right before execute()
         */
        virtual void undo() = 0;

        /**
         * @brief A short label for the action, e.g. for an "Undo <X>" menu item
         */
        [[nodiscard]] virtual const char* name() const { return "Action"; }
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_COMMAND_H
