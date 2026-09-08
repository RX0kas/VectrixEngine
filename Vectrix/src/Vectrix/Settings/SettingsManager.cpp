#include "SettingsManager.h"

#include "Vectrix/Application.h"

namespace Vectrix {
	JsonObject& SettingsManager::getSettings() {
		return Application::getSettingsManager().m_settings;
	}

	std::pair<VectrixResult, std::string> SettingsManager::load(const std::filesystem::path& file) {
		if (!std::filesystem::exists(file)) {
			// No settings file yet: adopt the path so a later save() can create it,
			// and start from empty settings.
			m_settings.clear();
			m_filePath = file;
			return {SUCCESS, ""};
		}

		auto [result, root] = Json::load(file.string());
		if (result != SUCCESS) {
			return {result, "Error while parsing settings: " + root.getString()};
		}

		if (!root.isType<JsonObject>()) {
			return {WRONG_TYPE, "Settings root is not a JSON object"};
		}

		// Only now that the load has fully succeeded do we replace the current state.
		m_settings = root.getAsObject();
		m_filePath = file;
		return {SUCCESS, ""};
	}

	std::pair<VectrixResult, std::string> SettingsManager::save() const {
		if (m_filePath.empty()) {
			// Temporary: no project is open yet (the project-selection screen is not
			// implemented), so there is nowhere project-scoped to persist to. Treat this
			// as a no-op instead of an error.
			return {SUCCESS, ""};
		}

		const VectrixResult result = Json::save(m_filePath.string(), m_settings);
		if (result != SUCCESS) {
			return {result, "Error while saving settings: " + toString(result)};
		}
		return {SUCCESS, ""};
	}
} // Vectrix
