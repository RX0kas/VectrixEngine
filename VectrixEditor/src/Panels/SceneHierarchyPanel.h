#ifndef VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#define VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#include <memory>

#include "imgui.h"
#include "Vectrix/Scene/Entity.h"

namespace Vectrix {
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const std::shared_ptr<Scene>& scene);

        void setContext(const std::shared_ptr<Scene>& scene);

        void onImGuiRender();
    private:
        void drawEntityNode(Entity& entity);
        void drawProperties(Entity entity);
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