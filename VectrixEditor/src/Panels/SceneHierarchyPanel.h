#ifndef VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#define VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#include <memory>

#include "imgui.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() : m_selectionContext(Entity::nullEntity()) {}

        SceneHierarchyPanel(const std::shared_ptr<Scene>& scene);

        void setContext(const std::shared_ptr<Scene>& scene);

        void onImGuiRender();

        Entity& getSelectedEntity() { return m_selectionContext; }
        void setSelectedEntity(Entity entity) { m_selectionContext = entity; }
        void resetSelectedEntity() { m_selectionContext = Entity::nullEntity(); }
    private:
        void drawEntityNode(Entity& entity);
        static void drawProperties(Entity entity);
        static bool drawTreeNodeComponent(const std::string& text, bool& mustBeRemoved, bool removable=true);

        static void showField(const char* label, const auto& ptr, const std::string& name) {
            if (ptr != nullptr)
                ImGui::Text("%s: %s", label, name.c_str());
            else
                ImGui::TextDisabled("%s: nullptr", label);
        }
    private:
        std::shared_ptr<Scene> m_context;
        Entity m_selectionContext;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H