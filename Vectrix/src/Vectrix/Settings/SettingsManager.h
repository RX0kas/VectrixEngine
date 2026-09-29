#ifndef VECTRIXWORKSPACE_SETTINGS_H
#define VECTRIXWORKSPACE_SETTINGS_H
#include <filesystem>

#include "Vectrix/Utils/Json.h"
#include "Vectrix/Utils/Result.h"

namespace Vectrix {
	class Application;

	constexpr auto settingsFileName = "settings.vectrix.json";

	/**
	 * @brief Two-tier settings store owned by the Application.
	 *
	 * A @c Global tier holds machine-wide preferences (loaded once at start-up); a
	 * @c Project tier holds per-project overrides (loaded when a project is opened, empty
	 * otherwise). getSettings() returns the two merged together, project winning key by
	 * key, which is what the rest of the engine reads. The settings UI writes into one
	 * tier at a time and saves that tier's file.
	 * @ingroup settings
	 */
	class SettingsManager {
	public:
		enum class Scope { Global, Project };

		/**
		 * @brief Replace the global tier from @p file (its path is adopted even if absent).
		 */
		[[nodiscard]] std::pair<VectrixResult, std::string> loadGlobal(const std::filesystem::path& file);

		/**
		 * @brief Replace the project tier from @p file; an empty path or a failed load
		 *        clears the project tier so the store falls back to the global one.
		 */
		[[nodiscard]] std::pair<VectrixResult, std::string> loadProject(const std::filesystem::path& file);

		/**
		 * @brief Write one tier back to its file.
		 */
		[[nodiscard]] std::pair<VectrixResult, std::string> save(Scope scope) const;

		/**
		 * @brief Whether a project tier is currently loaded.
		 */
		[[nodiscard]] static bool hasProject();

		/**
		 * @brief The merged settings (global, then project overrides). Read-only.
		 */
		[[nodiscard]] static const JsonObject& getSettings();

		/**
		 * @brief The editable object for one tier. Call markChanged() after mutating it.
		 */
		[[nodiscard]] static JsonObject& tier(Scope scope);

		/**
		 * @brief Invalidate the cached merged view so the next getSettings() rebuilds it.
		 */
		static void markChanged();

	private:
		friend class Application;

		[[nodiscard]] std::pair<VectrixResult, std::string> loadInto(JsonObject& target, const std::filesystem::path& file);
		void rebuildEffective() const;

		JsonObject m_global;
		JsonObject m_project;
		std::filesystem::path m_globalPath;
		std::filesystem::path m_projectPath;

		mutable JsonObject m_effective;
		mutable bool m_effectiveDirty = true;

		SettingsManager() = default;
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_SETTINGS_H
