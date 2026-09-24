#include "UndoHistory.h"

namespace Vectrix {
    Command* UndoHistory::push(std::unique_ptr<Command> command) {
        command->execute();
        Command* raw = command.get();

        m_redoStack.clear();
        m_undoStack.push_back(std::move(command));
        if (m_undoStack.size() > m_maxDepth)
            m_undoStack.pop_front();

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
