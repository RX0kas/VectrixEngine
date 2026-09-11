#include "SettingsManager.h"

#include "Vectrix/Application.h"

namespace Vectrix {
	namespace {
		void mergeInto(JsonObject& base, const JsonObject& overlay) {
			for (const auto& [key, overlayValue] : overlay) {
				const auto baseIt = base.find(key);
				if (baseIt != base.end() && baseIt->second.isType<JsonObject>() && overlayValue.isType<JsonObject>()) {
					JsonObject merged = baseIt->second.getAsObject();
					mergeInto(merged, overlayValue.getAsObject());
					baseIt->second = JsonValue(merged);
				} else {
					base[key] = overlayValue;
				}
			}
		}
	}

	const JsonObject& SettingsManager::getSettings() {
		SettingsManager& self = Application::getSettingsManager();
		if (self.m_effectiveDirty)
			self.rebuildEffective();
		return self.m_effective;
	}

	JsonObject& SettingsManager::tier(const Scope scope) {
		SettingsManager& self = Application::getSettingsManager();
		return scope == Scope::Project ? self.m_project : self.m_global;
	}

	void SettingsManager::markChanged() {
		Application::getSettingsManager().m_effectiveDirty = true;
	}

	bool SettingsManager::hasProject() {
		return !Application::getSettingsManager().m_projectPath.empty();
	}

	void SettingsManager::rebuildEffective() const {
		m_effective = m_global;
		mergeInto(m_effective, m_project);
		m_effectiveDirty = false;
	}

	std::pair<VectrixResult, std::string> SettingsManager::loadInto(JsonObject& target, const std::filesystem::path& file) {
		if (!std::filesystem::exists(file)) {
			target.clear();
			return {SUCCESS, ""};
		}

		auto [result, root] = Json::load(file.string());
		if (result != SUCCESS)
			return {result, "Error while parsing settings: " + root.getString()};
		if (!root.isType<JsonObject>())
			return {WRONG_TYPE, "Settings root is not a JSON object"};

		target = root.getAsObject();
		return {SUCCESS, ""};
	}

	std::pair<VectrixResult, std::string> SettingsManager::loadGlobal(const std::filesystem::path& file) {
		m_effectiveDirty = true;
		m_globalPath = file; // adopted even when absent, so the first save can create it
		return loadInto(m_global, file);
	}

	std::pair<VectrixResult, std::string> SettingsManager::loadProject(const std::filesystem::path& file) {
		m_effectiveDirty = true;

		if (file.empty()) {
			m_project.clear();
			m_projectPath.clear();
			return {SUCCESS, ""};
		}

		const auto outcome = loadInto(m_project, file);
		m_projectPath = (outcome.first == SUCCESS) ? file : std::filesystem::path{};
		return outcome;
	}

	std::pair<VectrixResult, std::string> SettingsManager::save(const Scope scope) const {
		const std::filesystem::path& path = scope == Scope::Project ? m_projectPath : m_globalPath;
		const JsonObject& data = scope == Scope::Project ? m_project : m_global;

		if (path.empty())
			return {NOT_FOUND, scope == Scope::Project ? "No project is open" : "No global settings location is set"};

		const VectrixResult result = Json::save(path.string(), data);
		if (result != SUCCESS)
			return {result, "Error while saving settings: " + toString(result)};
		return {SUCCESS, ""};
	}
} // Vectrix
