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
		static std::vector<RecentProject> loadRecentProject();
	private:
		std::string m_pendingOpenProjectPath;
		std::filesystem::path m_pendingOpenScenePath;
		std::string m_newProjectName = "Project";
		std::filesystem::path m_projectDirectory;
		std::string m_lastError;
		bool m_loadNewProject = false;
		bool m_hasCustomPath = false;

		/// @see RecentProject
		std::vector<RecentProject> m_recentProjects;

		void showOpenDialog();
		void pickFolder();
		void switchToEditor(const std::filesystem::path& projectDirectory, const std::filesystem::path& startScenePath);
		void openRecentProject(const RecentProject& project);
		void addRecentProject(RecentProject project);
		void launchFromFile(const std::filesystem::path& file);
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_STARTUPLAYER_H
