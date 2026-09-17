#include "StartupLayer.h"

#include <complex>

#include "EditorLayer.h"
#include "imgui.h"
#include "nfd.h"
#include "Vectrix/Application.h"
#include "Vectrix/Project/ProjectSerializer.h"
#include "Vectrix/Utils/Folders.h"

namespace Vectrix {
	StartupLayer::StartupLayer() : Layer("StartupLayer") {
		m_recentProjects = loadRecentProject();
	}

	StartupLayer::~StartupLayer() = default;

	void StartupLayer::OnUpdate(const DeltaTime &deltaTime) {
		if (m_loadNewProject) {
			m_loadNewProject = false;

			const std::filesystem::path target = m_hasCustomPath? m_projectDirectory : DefaultVectrixProjectPath / m_newProjectName;

			VC_CORE_INFO("Create project requested: \"{}\" in {}", m_newProjectName.c_str(), target.string().c_str());
			const auto [result, message] = ProjectSerializer::createProject(target, m_newProjectName);
			if (result != SUCCESS) {
				VC_CORE_ERROR_NO_EXIT("Can't create project: {}", message.c_str());
				m_lastError = message;
				return;
			}

			addRecentProject({m_newProjectName, target / (m_newProjectName + ".vcproj")});
			switchToEditor(target, std::filesystem::path("Scenes") / (m_newProjectName + ".vctx"));
		}

		if (!m_pendingOpenProjectPath.empty()) {
			const std::filesystem::path projectFile = m_pendingOpenProjectPath;
			m_pendingOpenProjectPath.clear();

			VC_CORE_INFO("Open project requested: {}", projectFile.string().c_str());
			const ProjectLoadResult loaded = ProjectSerializer::loadProject(projectFile);
			if (loaded.result != SUCCESS) {
				VC_CORE_ERROR_NO_EXIT("Can't open project: {}", toString(loaded.result).c_str());
				m_lastError = "Can't open project: " + toString(loaded.result);
				return;
			}

			addRecentProject({loaded.name, projectFile});
			switchToEditor(projectFile.parent_path(), loaded.startScenePath);
		}
	}

	void StartupLayer::switchToEditor(const std::filesystem::path& projectDirectory, const std::filesystem::path& startScenePath) {
		VC_CORE_INFO("Switching to EditorLayer, project: {}, starting scene: {}", projectDirectory.string().c_str(), startScenePath.string().c_str());
		JsonObject data;
		data["scenePath"] = JsonValue((projectDirectory / startScenePath).string());
		Application::instance().switchToLayer<EditorLayer>(this, data);
	}

	void StartupLayer::openRecentProject(const RecentProject& project) {
		VC_CORE_INFO("Recent project selected: \"{}\" ({})", project.name.c_str(), project.path.string().c_str());
		m_pendingOpenProjectPath = project.path.string();
	}

	void StartupLayer::showOpenDialog() {
		NFD_Init();

		nfdchar_t* outPath;
		nfdfilteritem_t filters[] = { { "Vectrix Project", "vcproj" } };

		nfdresult_t result = NFD_OpenDialog(&outPath, filters, 1, nullptr);

		if (result == NFD_OKAY) {
			m_pendingOpenProjectPath = std::string(outPath);
			VC_CORE_INFO("Project file picked: {}", m_pendingOpenProjectPath.c_str());
			NFD_FreePath(outPath);
		} else if (result == NFD_CANCEL) {
			VC_CORE_INFO("User cancelled");
		} else {
			VC_CORE_ERROR_NO_EXIT("NFD Error: {}", NFD_GetError());
		}

		NFD_Quit();
	}

	void StartupLayer::pickFolder() {
		NFD_Init();

		nfdchar_t* outPath;
		nfdresult_t result = NFD_PickFolder(&outPath,DefaultVectrixProjectPath.c_str());

		if (result == NFD_OKAY) {
			const std::filesystem::path scenePath(outPath);
			m_projectDirectory = scenePath;
			m_hasCustomPath = true;
			VC_CORE_INFO("Project folder picked: {}", scenePath.string().c_str());
			NFD_FreePath(outPath);
		} else if (result == NFD_CANCEL) {
			VC_CORE_INFO("User cancelled");
		} else {
			VC_CORE_ERROR_NO_EXIT("NFD Error: {}", NFD_GetError());
		}

		NFD_Quit();
	}

	void StartupLayer::OnAttach() {

	}

	void StartupLayer::OnAttach(const JsonObject &data) {
		if (data.contains("openProject")) {
			loadRecentProject();
			for (const auto&[name,path] : m_recentProjects) {
				if (path == data.at("openProject").getString()) {
					m_pendingOpenProjectPath = path;
				}
			}
		}
	}

	void StartupLayer::OnImGuiRender() {
		ImGui::Begin("Vectrix", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::BeginChild("##recentProjects", ImVec2(220.0f, 260.0f), true);
		ImGui::TextDisabled("Recent projects");
		ImGui::Separator();
		if (m_recentProjects.empty()) {
			ImGui::TextWrapped("No recent projects yet.");
		} else {
			for (const RecentProject& project : m_recentProjects) {
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

			const std::filesystem::path displayedPath = m_hasCustomPath
				? m_projectDirectory
				: DefaultVectrixProjectPath / m_newProjectName;
			ImGui::TextUnformatted(displayedPath.string().c_str());
			ImGui::SameLine();
			if (ImGui::Button("Choose folder")) {
				pickFolder();
			}

			if (!m_lastError.empty()) {
				ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", m_lastError.c_str());
			}

			if (ImGui::ColorButton("Create", ImVec4(0,255,0,255))) {
				m_newProjectName = name;
				m_lastError.clear();
				m_loadNewProject = true;
			}

			ImGui::EndPopup();
		}
		ImGui::EndChild();

		ImGui::End();
	}

	// TODO: add cache
	std::vector<RecentProject> StartupLayer::loadRecentProject() {
		std::filesystem::create_directories(getVectrixStateFolder());
		std::filesystem::path recentProjectsFile = getVectrixStateFolder() /  "recentProjects.json";
		std::vector<RecentProject> recentProjects = {};
		if (std::filesystem::exists(recentProjectsFile)) {
			VC_CORE_INFO("Recent project file found, loading recent projects");
			auto result = Json::load(recentProjectsFile);
			if (result.first!=SUCCESS) {
				VC_CORE_ERROR_NO_EXIT("Can't load recent projects, JSON error : {}",toString(result.first ));
				return {};
			}

			if (!result.second.isType<JsonObject>()) {
				VC_CORE_ERROR_NO_EXIT("Wrong formating in recent project file");
				return {};
			}
			try {
				for (const auto& p : result.second["projects"].asArray()) {
					recentProjects.push_back({p["name"].getString(),p["path"].getString()});
				}
			} catch (const std::exception& e) {
				VC_CORE_ERROR_NO_EXIT("Error while loading recent project file: {}",e.what());
			}
		} else {
			const JsonObject recentProjectsObj = { {"projects", JsonArray{}} };
			auto result = Json::save(recentProjectsFile, recentProjectsObj);
			if (result!=SUCCESS) {
				VC_CORE_ERROR_NO_EXIT("Error while saving recent project file: {}",toString(result));
				return {};
			}
		}
		return recentProjects;
	}

	void StartupLayer::addRecentProject(RecentProject project) {
		std::filesystem::create_directories(getVectrixStateFolder());
		std::filesystem::path recentProjectsFile = getVectrixStateFolder() / "recentProjects.json";

		JsonValue root;
		if (std::filesystem::exists(recentProjectsFile)) {
			VC_CORE_INFO("Adding project {} to recent projects file", project.name);

			auto result = Json::load(recentProjectsFile);
			if (result.first!=SUCCESS) {
				VC_CORE_ERROR_NO_EXIT("Can't load recent projects, JSON error : {}",toString(result.first));
				return;
			}

			if (!result.second.isType<JsonObject>()) {
				VC_CORE_ERROR_NO_EXIT("Wrong formating in recent project file");
				return;
			}

			root = std::move(result.second);
		} else {
			VC_CORE_INFO("Creating recent projects file at {}",recentProjectsFile.string());
		}

		const std::string pathString = project.path.string();
		JsonArray& projects = root["projects"].asArray();
		for (const auto& p : projects) {
			if (p["path"].getString() == pathString) {
				VC_CORE_INFO("Project {} already in recent projects file", project.name);
				return;
			}
		}

		JsonObject entry;
		entry.emplace("name", project.name);
		entry.emplace("path", pathString);
		projects.emplace_back(entry);

		const VectrixResult result = Json::save(recentProjectsFile, root.asObject());
		if (result!=SUCCESS) {
			VC_CORE_ERROR_NO_EXIT("Can't add project {} to recent projects file: {}",project.name, toString(result));
			return;
		}

		VC_CORE_INFO("Project {} added to recent projects file", project.name);
		m_recentProjects.push_back(std::move(project));
	}
} // Vectrix