#ifndef VECTRIXWORKSPACE_SETTINGS_H
#define VECTRIXWORKSPACE_SETTINGS_H
#include <filesystem>

#include "Vectrix/Utils/Json.h"
#include "Vectrix/Utils/Result.h"

namespace Vectrix {
	class Application;

	constexpr auto settingsFileName = "settings.vectrix.json";

	/**
	 * @brief In-memory store for the project settings, owned by the Application
	 *
	 * It holds the parsed settings tree and knows which file it came from so it can be
	 * written back. It has no notion of scenes or the editor: the caller supplies the
	 * path to load from.
	 * @ingroup settings
	 */
	class SettingsManager {
	public:
		/**
		 * @brief Parse @p file and replace the in-memory settings with its contents.
		 * @param file Path to the settings JSON file.
		 * @return SUCCESS, or a result and message describing why the load failed.
		 */
		[[nodiscard]] std::pair<VectrixResult, std::string> load(const std::filesystem::path& file);

		/**
		 * @brief Write the in-memory settings back to the file they were loaded from.
		 * @return SUCCESS, or a result and message describing why the save failed.
		 */
		[[nodiscard]] std::pair<VectrixResult, std::string> save() const;

		/**
		 * @brief Access the settings tree of the running application.
		 */
		[[nodiscard]] static JsonObject& getSettings();

	private:
		friend class Application;

		JsonObject m_settings;
		std::filesystem::path m_filePath;

		SettingsManager() = default;
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_SETTINGS_H
