#include "SettingsPanel.h"

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Vectrix/Settings/Outline.h"

namespace Vectrix {
    SettingsPanel::SettingsPanel() : ImGuiWidget("SettingsPanel") {

    }

    void SettingsPanel::render() {
        if (!ImGui::Begin(getName().c_str(),&getEnable())) {
            ImGui::End();
            return;
        }
        ImGui::SeparatorText("Outline");
        ImGui::DragFloat("Thickness",&outlineSettings.thickness,0.1f,0.0f,64.0f);
        ImGui::DragFloat2("Texel Size",glm::value_ptr(outlineSettings.texelSize),0.1f,0.0f,64.0f);
        ImGui::ColorEdit4("Color",glm::value_ptr(outlineSettings.color));

        ImGui::End();
    }
} // Vectrix
