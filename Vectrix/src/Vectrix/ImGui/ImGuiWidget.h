#ifndef VECTRIXWORKSPACE_IMGUIWIDGET_H
#define VECTRIXWORKSPACE_IMGUIWIDGET_H

#include "vcpch.h"

namespace Vectrix {
    class ImGuiWidget {
    public:
        ImGuiWidget(const std::string& name = "Widget");
        virtual ~ImGuiWidget();
        virtual void render();
        [[nodiscard]] const std::string& getName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };
}
#endif //VECTRIXWORKSPACE_IMGUIWIDGET_H