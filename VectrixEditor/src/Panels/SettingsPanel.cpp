#include "SettingsPanel.h"

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

#include "Vectrix/Application.h"
#include "Vectrix/Settings/Outline.h"
#include "Vectrix/Settings/SettingsManager.h"

namespace Vectrix {
    SettingsPanel::SettingsPanel() : ImGuiWidget("SettingsPanel") {}

    void SettingsPanel::render() {
        if (!ImGui::Begin(getName().c_str(), &getEnable())) {
            ImGui::End();
            return;
        }

        // The renderer reads outlineSettings every frame, so editing it here gives
        // immediate feedback; the settings file is only rewritten once an edit ends.
        bool commit = false;

        ImGui::SeparatorText("Outline");

        ImGui::DragFloat("Thickness", &outlineSettings.thickness, 0.1f, 0.0f, 64.0f);
        commit |= ImGui::IsItemDeactivatedAfterEdit();

        ImGui::DragFloat2("Texel Size", glm::value_ptr(outlineSettings.texelSize), 0.1f, 0.0f, 64.0f);
        commit |= ImGui::IsItemDeactivatedAfterEdit();

        ImGui::ColorEdit4("Color", glm::value_ptr(outlineSettings.color));
        commit |= ImGui::IsItemDeactivatedAfterEdit();

        if (commit) {
            writeOutlineSettings(SettingsManager::getSettings()["editor"]["outline"]);
            if (const auto [result, message] = Application::getSettingsManager().save(); result != SUCCESS)
                VC_ERROR_NO_EXIT("Could not save settings: {}", message);
        }

        ImGui::End();
    }
} // Vectrix
