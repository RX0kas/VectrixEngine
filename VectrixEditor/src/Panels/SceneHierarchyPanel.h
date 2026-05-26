#ifndef VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#define VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H
#include <memory>

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
        static void drawProperties(Entity entity);
    private:
        std::shared_ptr<Scene> m_context;
        Entity m_selectionContext;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SCENEHIERARCHYPANEL_H