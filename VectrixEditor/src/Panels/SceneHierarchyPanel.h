#ifndef VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#define VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#include <memory>

#include "imgui.h"
#include "Vectrix/ImGui/ImGuiWidget.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
    class SceneHierarchyPanel : public ImGuiWidget {
    public:
        SceneHierarchyPanel() : ImGuiWidget("SceneHierarchyPanel"){}

        SceneHierarchyPanel(const std::shared_ptr<Scene>& scene);

        void setContext(const std::shared_ptr<Scene>& scene);

        void render() override;

        std::shared_ptr<Entity> getSelectedEntity() { return m_selectionContext; }
        void setSelectedEntity(const std::shared_ptr<Entity>& entity) { m_selectionContext = entity; }
        void resetSelectedEntity() { m_selectionContext = nullptr; }
    private:
        void drawEntityNode(const std::shared_ptr<Entity>& entity);
        static void drawProperties(const std::shared_ptr<Entity>& entity);
        static bool drawTreeNodeComponent(const std::string& text, bool& mustBeRemoved, bool removable=true);

        static void showField(const char* label, const auto& ptr, const std::string& name) {
            if (ptr != nullptr)
                ImGui::Text("%s: %s", label, name.c_str());
            else
                ImGui::TextDisabled("%s: nullptr", label);
        }

        std::shared_ptr<Scene> m_context;
        std::shared_ptr<Entity> m_selectionContext;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H