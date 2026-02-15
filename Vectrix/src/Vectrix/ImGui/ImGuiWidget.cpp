#include "ImGuiWidget.h"

#include <utility>

namespace Vectrix {
    ImGuiWidget::ImGuiWidget(std::string name) : m_DebugName(std::move(name)) {

    }

    ImGuiWidget::~ImGuiWidget() = default;
}
