#ifndef VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#define VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#include <memory>

#include "imgui.h"
#include "Vectrix/ImGui/ImGuiWidget.h"
#include "Vectrix/Scene/Entity.h"
#include "Undo/UndoHistory.h"
#include "Undo/EntitySnapshot.h"

namespace Vectrix {
    class SceneHierarchyPanel : public ImGuiWidget {
    public:
        SceneHierarchyPanel() : ImGuiWidget("SceneHierarchyPanel"){}

        SceneHierarchyPanel(const std::shared_ptr<Scene>& scene);

        void setContext(const std::shared_ptr<Scene>& scene);

        /// Non-owning: EditorLayer, which owns both this panel and the UndoHistory, outlives it
        void setUndoHistory(UndoHistory* history) { m_undoHistory = history; }

        void render() override;

        std::shared_ptr<Entity> getSelectedEntity() { return m_selectionContext; }
        void setSelectedEntity(const std::shared_ptr<Entity>& entity) { m_selectionContext = entity; }
        void resetSelectedEntity() { m_selectionContext = nullptr; }

        /// Push a DeleteEntityCommand for the current selection, if any; a no-op otherwise
        void deleteSelectedEntity();
    private:
        void drawEntityNode(const std::shared_ptr<Entity>& entity);
        void drawProperties(const std::shared_ptr<Entity>& entity);
        static bool drawTreeNodeComponent(const std::string& text, bool& mustBeRemoved, bool removable=true);

        /// Pushes `command` through m_undoHistory, then re-selects whichever entity it now
        /// targets if it is a create/delete command (see EntityHandleCommand)
        void pushAndRefreshSelection(std::unique_ptr<Command> command);

        static void showField(const char* label, const auto& ptr, const std::string& name) {
            if (ptr != nullptr)
                ImGui::Text("%s: %s", label, name.c_str());
            else
                ImGui::TextDisabled("%s: nullptr", label);
        }

        std::shared_ptr<Scene> m_context;
        std::shared_ptr<Entity> m_selectionContext;
        UndoHistory* m_undoHistory = nullptr;

        // Scratch state for the "capture on activate, commit on deactivate" drag idiom used
        // by the property fields below, so a whole drag gesture becomes one undo step.
        TransformSnapshot m_transformDragBefore;
        float m_floatDragBefore = 0.0f;
        std::string m_nameEditBefore;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H