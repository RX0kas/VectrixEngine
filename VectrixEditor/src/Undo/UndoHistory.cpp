#include "UndoHistory.h"

namespace Vectrix {
    Command* UndoHistory::push(std::unique_ptr<Command> command) {
        command->execute();
        Command* raw = command.get();

        // The saved state was undone past: it sits in the redo stack, which a new action discards
        if (m_cleanIndex && *m_cleanIndex > m_undoStack.size())
            m_cleanIndex.reset();

        m_redoStack.clear();
        m_undoStack.push_back(std::move(command));
        if (m_undoStack.size() > m_maxDepth) {
            m_undoStack.pop_front();
            // Indices shift down with the dropped entry; at 0 the saved state was the one it undid to
            if (m_cleanIndex) {
                if (*m_cleanIndex == 0) m_cleanIndex.reset();
                else --*m_cleanIndex;
            }
        }

        return raw;
    }

    Command* UndoHistory::undo() {
        if (m_undoStack.empty())
            return nullptr;

        std::unique_ptr<Command> command = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        command->undo();

        Command* raw = command.get();
        m_redoStack.push_back(std::move(command));
        return raw;
    }

    Command* UndoHistory::redo() {
        if (m_redoStack.empty())
            return nullptr;

        std::unique_ptr<Command> command = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        command->execute();

        Command* raw = command.get();
        m_undoStack.push_back(std::move(command));
        return raw;
    }
} // Vectrix
