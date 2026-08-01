#ifndef VECTRIXWORKSPACE_SETTINGSPANEL_H
#define VECTRIXWORKSPACE_SETTINGSPANEL_H
#include "Vectrix/ImGui/ImGuiWidget.h"

namespace Vectrix {
    class SettingsPanel : public ImGuiWidget {
    public:
        SettingsPanel();
        void render() override;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SETTINGSPANEL_H