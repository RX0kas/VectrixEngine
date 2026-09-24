#ifndef VECTRIXWORKSPACE_ERROR_H
#define VECTRIXWORKSPACE_ERROR_H
#include <functional>
#include <string>

#include "imgui.h"

namespace Vectrix {
	static void showErrorMessage(const char* id) {
		ImGui::OpenPopup(id);
	}

	static void renderErrorMessage(const char* id, const std::string& errorMessage, const std::function<void()>& displayFunction) {
		static bool errorShowed = false;
		if (ImGui::BeginPopup(id)) {
			if (!errorShowed) {
				errorShowed = true;
				VC_ERROR_NO_EXIT(errorMessage);
			}
			ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", errorMessage.c_str());
			ImGui::Separator();
			displayFunction();
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
				errorShowed = false;
			}
			ImGui::EndPopup();
		}
	}

	static void renderErrorMessage(const char* id, const std::string& errorMessage) {
		static bool errorShowed = false;
		if (!errorShowed) {
			errorShowed = true;
			VC_ERROR_NO_EXIT(errorMessage);
		}
		if (ImGui::BeginPopup(id)) {
			ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", errorMessage.c_str());
			ImGui::Separator();
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
				errorShowed = false;
			}
			ImGui::EndPopup();
		}
	}
}

#endif //VECTRIXWORKSPACE_ERROR_H
