#include "ImGuiWidget.h"

#include <utility>

#include "Vectrix/Application.h"

namespace Vectrix {
    bool ImGuiWidget::vc_defaultEnable = false;

    ImGuiWidget::ImGuiWidget(std::string name) : m_DebugName(std::move(name)) {
        Application::instance().imguiLayer().addWidget(this);
    }

    ImGuiWidget::~ImGuiWidget() {
        Application::instance().imguiLayer().removeWidget(this);
    }
}
