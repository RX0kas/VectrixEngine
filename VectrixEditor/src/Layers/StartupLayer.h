#ifndef VECTRIXWORKSPACE_STARTUPLAYER_H
#define VECTRIXWORKSPACE_STARTUPLAYER_H
#include <filesystem>
#include <vector>

#include "Vectrix/Layers/Layer.h"

namespace Vectrix {
	/**
	 * @brief One entry of the "recent projects" list shown by StartupLayer
	 */
	struct RecentProject {
		/// Name shown in the list, typically the project's name from its .vcProj file
		std::string name;

		/// Path to the project's .vcProj file
		std::filesystem::path path;
	};

	class StartupLayer : public Layer {
	public:
		/**
		 * @param launchFile A .vcproj or .vctx path the OS was told to open us with
		 *                   (e.g. via a double-click file association), or empty
		 */
		explicit StartupLayer(const std::filesystem::path& launchFile = {});
		~StartupLayer() override;
		void OnImGuiRender() override;
		void OnAttach() override;
		void OnAttach(const JsonObject& data) override;
		void OnUpdate(const DeltaTime &deltaTime) override;
		/**
		 * @brief Returns the recent projects list, cached in memory
		 *
		 * recentProjects.json is only re-read when its last write time changed since the previous call.
		 * The returned reference stays valid until the next call that has to reload the file.
		 */
		static const std::vector<RecentProject>& loadRecentProject();
		static std::string& lastLoadRecentProjectsError();
	private:
		std::string m_pendingOpenProjectPath;
		std::filesystem::path m_pendingOpenScenePath;
		std::string m_newProjectName = "Project";
		std::filesystem::path m_projectDirectory;
		bool m_loadNewProject = false;
		bool m_hasCustomPath = false;

		static std::vector<RecentProject> readRecentProjectsFile(const std::filesystem::path& recentProjectsFile);

		void showOpenDialog();
		void pickFolder();
		void switchToEditor(const std::filesystem::path& projectDirectory, const std::filesystem::path& startScenePath);
		void openRecentProject(const RecentProject& project);
		void addRecentProject(const RecentProject& project);
		void launchFromFile(const std::filesystem::path& file);

		std::string m_lastCreateProjectErrorMessage;
		std::string m_lastLoadProjectErrorMessage;
		std::string m_lastOpenNFDErrorMessage;
		std::string m_lastOpenFolderNFDErrorMessage;
		std::string m_lastLaunchFromFileErrorMessage;
		std::string m_lastAddRecentProjectErrorMessage;
		std::string m_lastSaveRecentProjectErrorMessage;
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_STARTUPLAYER_H
