#ifndef VECTRIXWORKSPACE_IMGUIWIDGET_H
#define VECTRIXWORKSPACE_IMGUIWIDGET_H

#include "vcpch.h"

namespace Vectrix {
    class ImGuiWidget {
    public:
        ImGuiWidget(std::string name = "Widget");
        virtual ~ImGuiWidget() = 0;

        virtual void render() = 0;
        [[nodiscard]] const std::string& getName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };
}
#endif //VECTRIXWORKSPACE_IMGUIWIDGET_H