#include "ContentBrowserPanel.h"

#include "imgui.h"
#include "Vectrix/Assets/AssetsManager.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    static const std::filesystem::path s_AssetPath = "assets";

    static float padding = 16.0f;
    static float thumbnailSize = 128.0f;

    ContentBrowserPanel::ContentBrowserPanel() {
        if (std::filesystem::path(s_AssetPath).empty()) {
            VC_WARN("{} doesn't exist for the asset path",s_AssetPath.string());
            m_currentDirectory = std::filesystem::current_path();
        } else {
            m_currentDirectory = s_AssetPath;
        }

        auto d = AssetsManager::load<Texture>("icons/ContentBrowser/directory.png");
        if (d.first!=SUCCESS) {
            VC_ERROR("Can't load directory icon: {}",toString(d.first));
        }
        m_directoryIcon = d.second;

        auto f = AssetsManager::load<Texture>("icons/ContentBrowser/file.png");
        if (f.first!=SUCCESS) {
            VC_ERROR("Can't load file icon: {}",toString(f.first));
        }
        m_fileIcon = f.second;
    }

    void ContentBrowserPanel::onImGuiRender() {
        ImGui::Begin("Content Browser");

        if (m_currentDirectory != std::filesystem::path(s_AssetPath)) {
            if (ImGui::Button("<-"))
            {
                m_currentDirectory = m_currentDirectory.parent_path();
            }
        }

        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = static_cast<int>(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, nullptr, false);

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory)) {
            const auto& path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, s_AssetPath);
            std::string filenameString = relativePath.filename().string();

            ImGui::PushID(filenameString.c_str());
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

            if (directoryEntry.is_directory()) {
                drawFolder(path,relativePath);
            } else {
                drawFile(path,relativePath);
            }

            if (ImGui::BeginDragDropSource()) {
#ifdef VC_PLATFORM_LINUX
                const char* itemPath = relativePath.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (strlen(itemPath) + 1) * sizeof(char));
#else
                const wchar_t* itemPath = relativePath.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
#endif

                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();


            ImGui::TextWrapped("%s",relativePath.filename().c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);

        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        // TODO: status bar
        ImGui::End();
    }

    void ContentBrowserPanel::drawFile(const std::filesystem::path& path, const std::filesystem::path &relativePath) {
        std::shared_ptr<Texture> icon;
            // TODO: don't load multiple times
            AssetType type = AssetsManager::getAssetType(path);
            if (type==AssetType::TEXTURE) {
                auto t = AssetsManager::load<Texture>(relativePath);
                if (t.first!=SUCCESS) {
                    VC_ERROR_NO_EXIT("Can't load icon for {}: {}",relativePath.string(),toString(t.first));
                }
                icon = t.second;
            } else {
                icon = m_fileIcon;
            }

        ImGui::ImageButton(path.filename().c_str(),icon->getImGuiTextureID(), { thumbnailSize, thumbnailSize }, { 1, 0 }, { 0, 1 });

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {}
    }

    void ContentBrowserPanel::drawFolder(const std::filesystem::path &path, const std::filesystem::path &relativePath) {
        std::shared_ptr<Texture> icon = m_directoryIcon;

        ImGui::ImageButton(path.filename().c_str(),icon->getImGuiTextureID(), { thumbnailSize, thumbnailSize }, { 1, 0 }, { 0, 1 });

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            m_currentDirectory = path;
        }
    }
} // Vectrix