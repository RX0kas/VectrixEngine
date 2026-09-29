#ifndef VECTRIXWORKSPACE_ERROR_H
#define VECTRIXWORKSPACE_ERROR_H
#include <functional>
#include <string>
#include <unordered_set>

#include "imgui.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
	namespace Detail {
		/// Popups requested by showErrorMessage, waiting for their renderErrorMessage call to open them
		inline std::unordered_set<std::string>& pendingErrorPopups() {
			static std::unordered_set<std::string> pending;
			return pending;
		}

		/// Opens the popup if showErrorMessage requested it, and logs the message once at that moment
		inline void openPendingErrorPopup(const char* id, const std::string& errorMessage) {
			// Checked first so the per-frame calls don't build a std::string key when nothing is pending
			std::unordered_set<std::string>& pending = pendingErrorPopups();
			if (!pending.empty() && pending.erase(id) > 0) {
				VC_ERROR_NO_EXIT(errorMessage);
				ImGui::OpenPopup(id);
			}
		}
	}

	/**
	 * @brief Requests the error popup id to be opened
	 *
	 * The popup is opened by the next renderErrorMessage(id, ...) call rather than here: ImGui popup ids depend on
	 * the ID stack, so opening it here (e.g. from inside a menu or a tree node) would never match the BeginPopup
	 * done by renderErrorMessage. It also makes it safe to call outside an ImGui frame (e.g. from a constructor).
	 */
	inline void showErrorMessage(const char* id) {
		Detail::pendingErrorPopups().insert(id);
	}

	inline void renderErrorMessage(const char* id, const std::string& errorMessage, const std::function<void()>& displayFunction) {
		Detail::openPendingErrorPopup(id, errorMessage);
		if (ImGui::BeginPopup(id)) {
			ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", errorMessage.c_str());
			ImGui::Separator();
			displayFunction();
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	inline void renderErrorMessage(const char* id, const std::string& errorMessage) {
		Detail::openPendingErrorPopup(id, errorMessage);
		if (ImGui::BeginPopup(id)) {
			ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", errorMessage.c_str());
			ImGui::Separator();
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}

#endif //VECTRIXWORKSPACE_ERROR_H
