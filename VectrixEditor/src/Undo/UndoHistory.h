#ifndef VECTRIXWORKSPACE_UNDOHISTORY_H
#define VECTRIXWORKSPACE_UNDOHISTORY_H

#include <deque>
#include <memory>
#include <optional>

#include "Command.h"

/**
 * @file UndoHistory.h
 * @brief The undo/redo stack owned by the editor
 */

namespace Vectrix {
    /**
     * @brief Owns the undo and redo stacks of Command objects for one editing session
     *
     * A plain member of EditorLayer, not persisted (see Vectrix/Settings/SettingsManager.h
     * for the persisted settings store, which this deliberately does not go through) and
     * cleared whenever the active scene changes, since undoing across a scene swap makes
     * no sense.
     */
    class UndoHistory {
    public:
        explicit UndoHistory(size_t maxDepth = 100) : m_maxDepth(maxDepth) {}

        /**
         * @brief Execute a new command and make it the top of the undo stack
         * @param command The command to run; ownership moves to the history
         * @return The command just pushed, still owned by the history, for callers that
         *         need to inspect what just happened (e.g. via EntityHandleCommand)
         * @note Clears the redo stack: a fresh action invalidates any previously undone one
         */
        Command* push(std::unique_ptr<Command> command);

        /**
         * @brief Undo the most recent command, moving it onto the redo stack
         * @return The command just undone, or nullptr when the undo stack was empty
         */
        Command* undo();

        /**
         * @brief Redo the most recently undone command, moving it back onto the undo stack
         * @return The command just redone, or nullptr when the redo stack was empty
         */
        Command* redo();

        [[nodiscard]] bool canUndo() const { return !m_undoStack.empty(); }
        [[nodiscard]] bool canRedo() const { return !m_redoStack.empty(); }

        /// Label of the command undo() would revert, or nullptr when there is none
        [[nodiscard]] const char* peekUndoName() const {
            return m_undoStack.empty() ? nullptr : m_undoStack.back()->name();
        }

        /// Label of the command redo() would reapply, or nullptr when there is none
        [[nodiscard]] const char* peekRedoName() const {
            return m_redoStack.empty() ? nullptr : m_redoStack.back()->name();
        }

        /// Drop every entry in both stacks, e.g. when the active scene changes. The state left is the clean one
        void clear() {
            m_undoStack.clear();
            m_redoStack.clear();
            m_cleanIndex = 0;
        }

        /// Record the current state as the one saved on disk
        void markClean() { m_cleanIndex = m_undoStack.size(); }

        /// The scene was changed without a command (e.g. assets relinked when it was opened): no state reachable by
        /// undo or redo is the saved one anymore, until the next markClean
        void markModified() { m_cleanIndex.reset(); }

        /**
         * @brief Whether the current state is the one last saved (see markClean)
         * @note Undoing or redoing back to the saved state makes it clean again
         */
        [[nodiscard]] bool isClean() const { return m_cleanIndex == m_undoStack.size(); }

    private:
        std::deque<std::unique_ptr<Command>> m_undoStack;
        std::deque<std::unique_ptr<Command>> m_redoStack;
        size_t m_maxDepth;
        /// Undo stack size matching the saved state, nullopt once that state can't be reached anymore
        std::optional<size_t> m_cleanIndex = 0;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_UNDOHISTORY_H
