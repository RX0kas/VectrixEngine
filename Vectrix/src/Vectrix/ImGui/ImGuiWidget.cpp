#include "ImGuiWidget.h"

#include <utility>

namespace Vectrix {
    bool ImGuiWidget::vc_defaultEnable = false;

    ImGuiWidget::ImGuiWidget(std::string name) : m_DebugName(std::move(name)) {

    }

    ImGuiWidget::~ImGuiWidget() = default;
}
