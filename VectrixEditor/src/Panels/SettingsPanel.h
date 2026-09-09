#ifndef VECTRIXWORKSPACE_SETTINGSPANEL_H
#define VECTRIXWORKSPACE_SETTINGSPANEL_H
#include "Vectrix/ImGui/ImGuiWidget.h"

namespace Vectrix {
    /**
     * @brief Two-pane preferences window: a category list on the left, its controls on the right.
     *
     * Every control reads and writes the project settings tree through SettingsManager and
     * saves the file on change. Only the Outline section takes effect immediately; the rest
     * are consumed by the backend at start-up.
     */
    class SettingsPanel : public ImGuiWidget {
    public:
        SettingsPanel();
        void render() override;

    private:
        int m_selectedCategory = -1; ///< -1 until seeded from the settings file on the first render
        int m_saveScope = 0;         ///< 0 = Project tier, 1 = Global tier
        char m_search[64] = {};
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_SETTINGSPANEL_H
