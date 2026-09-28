#include "StartupLayer.h"

#include "EditorLayer.h"
#include "imgui.h"
#include "nfd.h"
#include "Utils/Error.h"
#include "Vectrix/Application.h"
#include "Vectrix/Project/ProjectSerializer.h"
#include "Vectrix/Settings/SettingsManager.h"
#include "Vectrix/Utils/Folders.h"

namespace Vectrix {
	StartupLayer::StartupLayer(const std::filesystem::path& launchFile) : Layer("StartupLayer") {
		launchFromFile(launchFile);
	}

	StartupLayer::~StartupLayer() = default;

	void StartupLayer::OnUpdate(const DeltaTime &deltaTime) {
		if (m_loadNewProject) {
			m_loadNewProject = false;

			const std::filesystem::path target = m_hasCustomPath? m_projectDirectory : DefaultVectrixProjectPath / m_newProjectName;

			VC_INFO("Create project requested: \"{}\" in {}", m_newProjectName.c_str(), target.string().c_str());
			const auto [result, message] = ProjectSerializer::createProject(target, m_newProjectName);
			if (result != SUCCESS) {
				showErrorMessage("ERROR_CREATING_PROJECT");
				m_lastCreateProjectErrorMessage = std::format("Can't create project: {}", message);
				return;
			}

			addRecentProject({m_newProjectName, target / (m_newProjectName + ".vcproj")});
			switchToEditor(target, std::filesystem::path("Scenes") / (m_newProjectName + ".vctx"));
		}

		if (!m_pendingOpenProjectPath.empty()) {
			const std::filesystem::path projectFile = m_pendingOpenProjectPath;
			m_pendingOpenProjectPath.clear();

			VC_INFO("Open project requested: {}", projectFile.string().c_str());
			const ProjectLoadResult loaded = ProjectSerializer::loadProject(projectFile);
			if (loaded.result != SUCCESS) {
				showErrorMessage("ERROR_LOADING_PROJECT");
				m_lastLoadProjectErrorMessage = std::format("Can't open project: {}", toString(loaded.result));
				return;
			}

			addRecentProject({loaded.name, projectFile});

			// A specific scene (opened via a .vctx file association) overrides the
			// project's default start scene.
			std::filesystem::path startScene = loaded.startScenePath;
			if (!m_pendingOpenScenePath.empty()) {
				startScene = std::filesystem::relative(m_pendingOpenScenePath, projectFile.parent_path());
				m_pendingOpenScenePath.clear();
			}

			switchToEditor(projectFile.parent_path(), startScene);
		}
	}

	void StartupLayer::switchToEditor(const std::filesystem::path& projectDirectory, const std::filesystem::path& startScenePath) {
		VC_INFO("Switching to EditorLayer, project: {}, starting scene: {}", projectDirectory.string().c_str(), startScenePath.string().c_str());
		JsonObject data;
		data["scenePath"] = JsonValue((projectDirectory / startScenePath).string());
		data["projectDirectory"] = JsonValue(projectDirectory.string());
		Application::instance().switchToLayer<EditorLayer>(this, data);
	}

	void StartupLayer::openRecentProject(const RecentProject& project) {
		VC_INFO("Recent project selected: \"{}\" ({})", project.name.c_str(), project.path.string().c_str());
		m_pendingOpenProjectPath = project.path.string();
	}

	void StartupLayer::showOpenDialog() {
		NFD_Init();

		nfdchar_t* outPath;
		nfdfilteritem_t filters[] = { { "Vectrix Project", "vcproj" } };

		nfdresult_t result = NFD_OpenDialog(&outPath, filters, 1, nullptr);

		if (result == NFD_OKAY) {
			m_pendingOpenProjectPath = std::string(outPath);
			VC_INFO("Project file picked: {}", m_pendingOpenProjectPath.c_str());
			NFD_FreePath(outPath);
		} else if (result == NFD_CANCEL) {
			VC_INFO("User cancelled");
		} else {
			showErrorMessage("OPEN_NFD_ERROR");
			m_lastOpenNFDErrorMessage = std::format("NFD Error: {}", NFD_GetError());
		}

		NFD_Quit();
	}

	void StartupLayer::pickFolder() {
		NFD_Init();

		nfdchar_t* outPath;
		nfdresult_t result = NFD_PickFolder(&outPath,DefaultVectrixProjectPath.string().c_str());

		if (result == NFD_OKAY) {
			const std::filesystem::path scenePath(outPath);
			m_projectDirectory = scenePath;
			m_hasCustomPath = true;
			VC_INFO("Project folder picked: {}", scenePath.string().c_str());
			NFD_FreePath(outPath);
		} else if (result == NFD_CANCEL) {
			VC_INFO("User cancelled");
		} else {
			showErrorMessage("OPEN_FOLDER_NFD_ERROR");
			m_lastOpenFolderNFDErrorMessage = std::format("NFD Error: {}", NFD_GetError());
		}

		NFD_Quit();
	}

	void StartupLayer::OnAttach() {

	}

	void StartupLayer::OnAttach(const JsonObject &data) {
		if (data.contains("openProject")) {
			// Loading a missing/invalid project is already reported by OnUpdate, no need to check it against the recent list
			m_pendingOpenProjectPath = data.at("openProject").getString();
		}
	}

	void StartupLayer::OnImGuiRender() {
		const std::vector<RecentProject>& recentProjects = loadRecentProject();

		ImGui::Begin("Vectrix", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::BeginChild("##recentProjects", ImVec2(220.0f, 260.0f), true);
		ImGui::TextDisabled("Recent projects");
		ImGui::Separator();
		if (recentProjects.empty()) {
			ImGui::TextWrapped("No recent projects yet.");
		} else {
			for (const RecentProject& project : recentProjects) {
				ImGui::PushID(&project);
				if (ImGui::Selectable(project.name.c_str())) {
					openRecentProject(project);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", project.path.string().c_str());
				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("##startupActions", ImVec2(260.0f, 260.0f), true);
		if (ImGui::Button("Open Project")) {
			showOpenDialog();
		}

		if (ImGui::Button("New project")) ImGui::OpenPopup("Create new project");

		if (ImGui::BeginPopupModal("Create new project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Create new project: ");
			static char name[256] = "Project";
			if (ImGui::InputText("ProjectName", name, 256)) {
				m_newProjectName = name;
			}

			const std::filesystem::path displayedPath = m_hasCustomPath ? m_projectDirectory : DefaultVectrixProjectPath / m_newProjectName;
			ImGui::TextUnformatted(displayedPath.string().c_str());
			ImGui::SameLine();
			if (ImGui::Button("Choose folder")) {
				pickFolder();
			}

			if (ImGui::Button("Create")) {
				m_newProjectName = name;
				m_loadNewProject = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				strcpy(name, "Project");
				m_newProjectName = "Project";
				m_hasCustomPath = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::EndChild();

		ImGui::End();

		renderErrorMessage("ERROR_CREATING_PROJECT", m_lastCreateProjectErrorMessage);
		renderErrorMessage("ERROR_LOADING_PROJECT",m_lastLoadProjectErrorMessage);
		renderErrorMessage("OPEN_FOLDER_NFD_ERROR",m_lastOpenFolderNFDErrorMessage);
		renderErrorMessage("OPEN_NFD_ERROR",m_lastOpenNFDErrorMessage);
		renderErrorMessage("LAUNCH_FROM_FILE_ERROR", m_lastLaunchFromFileErrorMessage);
		renderErrorMessage("ADD_RECENT_PROJECT_ERROR", m_lastAddRecentProjectErrorMessage);
		renderErrorMessage("SAVE_RECENT_PROJECT_ERROR", m_lastSaveRecentProjectErrorMessage);
		renderErrorMessage("LOAD_RECENT_PROJECTS_ERROR", lastLoadRecentProjectsError());
	}

	namespace {
		/// In-memory copy of recentProjects.json, shared by StartupLayer and EditorLayer
		struct RecentProjectsCache {
			std::vector<RecentProject> projects;
			std::filesystem::file_time_type lastWriteTime;
			bool loaded = false;
		};

		RecentProjectsCache& recentProjectsCache() {
			static RecentProjectsCache cache;
			return cache;
		}

		/// Computed once: loadRecentProject() runs every frame on the startup screen
		const std::filesystem::path& recentProjectsFilePath() {
			static const std::filesystem::path path = getVectrixStateFolder() / "recentProjects.json";
			return path;
		}

		std::filesystem::file_time_type lastWriteTimeOf(const std::filesystem::path& file) {
			std::error_code error;
			const std::filesystem::file_time_type time = std::filesystem::last_write_time(file, error);
			return error ? std::filesystem::file_time_type::min() : time;
		}
	}

	const std::vector<RecentProject>& StartupLayer::loadRecentProject() {
		const std::filesystem::path& recentProjectsFile = recentProjectsFilePath();
		RecentProjectsCache& cache = recentProjectsCache();
		if (cache.loaded && lastWriteTimeOf(recentProjectsFile) == cache.lastWriteTime) {
			return cache.projects;
		}

		cache.projects = readRecentProjectsFile(recentProjectsFile);
		// Taken after reading so a rewrite made by readRecentProjectsFile itself doesn't trigger a reload.
		// On error the empty list is cached too: a broken file is reported once, then again only after it changes.
		cache.lastWriteTime = lastWriteTimeOf(recentProjectsFile);
		cache.loaded = true;
		return cache.projects;
	}

	std::vector<RecentProject> StartupLayer::readRecentProjectsFile(const std::filesystem::path& recentProjectsFile) {
		std::filesystem::create_directories(getVectrixStateFolder());
		std::vector<RecentProject> recentProjects = {};
		if (std::filesystem::exists(recentProjectsFile)) {
			VC_INFO("Recent project file found, loading recent projects");
			auto result = Json::load(recentProjectsFile.string());
			if (result.first!=SUCCESS) {
				showErrorMessage("LOAD_RECENT_PROJECTS_ERROR");
				lastLoadRecentProjectsError() = std::format("Can't load recent projects, JSON error : {}",toString(result.first));
				return {};
			}

			if (!result.second.isType<JsonObject>()) {
				showErrorMessage("LOAD_RECENT_PROJECTS_ERROR");
				lastLoadRecentProjectsError() = "Wrong formating in recent project file";
				return {};
			}
			try {
				bool mustResave = false;
				for (const auto& p : result.second["projects"].asArray()) {
					if (std::filesystem::exists(std::filesystem::path(p["path"].getString())))
						recentProjects.push_back({p["name"].getString(),p["path"].getString()});
					else {
						VC_WARN("Project {} with path {} doesn't exist",p["name"].getString(),p["path"].getString());
						const JsonObject& settings = SettingsManager::getSettings();
						const auto editorSettings = settings.find("editor");
						const bool autoProjectRemove = editorSettings != settings.end()
							&& editorSettings->second["general"]["autoProjectRemove"].getAs<bool>().value_or(false);
						if (autoProjectRemove) {
							VC_INFO("Removing project {}",p["name"].getString());
							mustResave = true;
						} else {
							VC_WARN("If you want to remove it from the list, enable the setting autoProjectRemove");
						}
					}
				}
				if (mustResave) {
					JsonArray arr;
					for (const auto&[name, path] : recentProjects) {
						JsonObject proj;
						proj.emplace("name",name);
						proj.emplace("path", path.string());
						arr.emplace_back(proj);
					}

					const JsonObject recentProjectsObj = { {"projects", arr} };
					auto resultSaving = Json::save(recentProjectsFile.string(), recentProjectsObj);
					if (resultSaving!=SUCCESS) {
						showErrorMessage("LOAD_RECENT_PROJECTS_ERROR");
						lastLoadRecentProjectsError() = std::format("Failed to overwrite recent projects file: {}", toString(resultSaving));
					}
				}
			} catch (const std::exception& e) {
				showErrorMessage("LOAD_RECENT_PROJECTS_ERROR");
				lastLoadRecentProjectsError() = std::format("Error while loading recent project file: {}",e.what());
			}
		} else {
			const JsonObject recentProjectsObj = { {"projects", JsonArray{}} };
			auto result = Json::save(recentProjectsFile.string(), recentProjectsObj);
			if (result!=SUCCESS) {
				showErrorMessage("LOAD_RECENT_PROJECTS_ERROR");
				lastLoadRecentProjectsError() = std::format("Error while saving recent project file: {}",toString(result));
				return {};
			}
		}
		return recentProjects;
	}

	std::string& StartupLayer::lastLoadRecentProjectsError() {
		static std::string message;
		return message;
	}

	void StartupLayer::launchFromFile(const std::filesystem::path& file) {
		if (file.empty()) return;

		if (file.extension() == ".vcproj") {
			m_pendingOpenProjectPath = file.string();
			return;
		}

		if (file.extension() == ".vctx") {
			// Scenes live at <project>/Scenes/<name>.vctx, so the project's .vcproj
			// file is a sibling one directory up.
			const std::filesystem::path projectDir = file.parent_path().parent_path();
			if (std::filesystem::exists(projectDir)) {
				for (const auto& entry : std::filesystem::directory_iterator(projectDir)) {
					if (entry.path().extension() == ".vcproj") {
						m_pendingOpenProjectPath = entry.path().string();
						m_pendingOpenScenePath = file;
						return;
					}
				}
			}
			showErrorMessage("LAUNCH_FROM_FILE_ERROR");
			m_lastLaunchFromFileErrorMessage = std::format("Can't find a .vcproj next to scene: {}", file.string());
			return;
		}

		showErrorMessage("LAUNCH_FROM_FILE_ERROR");
		m_lastLaunchFromFileErrorMessage = std::format("Don't know how to open file: {}", file.string());
	}

	void StartupLayer::addRecentProject(const RecentProject& project) {
		std::filesystem::create_directories(getVectrixStateFolder());
		const std::filesystem::path& recentProjectsFile = recentProjectsFilePath();

		JsonValue root;
		if (std::filesystem::exists(recentProjectsFile)) {
			VC_INFO("Adding project {} to recent projects file", project.name);

			auto result = Json::load(recentProjectsFile.string());
			if (result.first!=SUCCESS) {
				showErrorMessage("ADD_RECENT_PROJECT_ERROR");
				m_lastAddRecentProjectErrorMessage = std::format("Can't load recent projects, JSON error : {}",toString(result.first));
				return;
			}

			if (!result.second.isType<JsonObject>()) {
				showErrorMessage("ADD_RECENT_PROJECT_ERROR");
				m_lastAddRecentProjectErrorMessage = "Wrong formating in recent project file";
				return;
			}

			root = std::move(result.second);
		} else {
			VC_INFO("Creating recent projects file at {}",recentProjectsFile.string());
		}

		const std::string pathString = project.path.string();
		JsonArray& projects = root["projects"].asArray();
		for (const auto& p : projects) {
			if (p["path"].getString() == pathString) {
				VC_INFO("Project {} already in recent projects file", project.name);
				return;
			}
		}

		JsonObject entry;
		entry.emplace("name", project.name);
		entry.emplace("path", pathString);
		projects.emplace_back(entry);

		const VectrixResult result = Json::save(recentProjectsFile.string(), root.asObject());
		if (result!=SUCCESS) {
			showErrorMessage("SAVE_RECENT_PROJECT_ERROR");
			m_lastSaveRecentProjectErrorMessage = std::format("Can't add project {} to recent projects file: {}",project.name, toString(result));
			return;
		}

		VC_INFO("Project {} added to recent projects file", project.name);
		// The file was re-read above and may hold entries the cache doesn't have yet, so reload it on next access
		recentProjectsCache().loaded = false;
	}
} // Vectrix