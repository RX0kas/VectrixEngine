#include "ContentBrowserPanel.h"

#include "imgui.h"
#include "imgui_internal.h"   // for ImGui::GetWindowDrawList()
#include "Vectrix/Assets/AssetsManager.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {

static void beginDragDropSource(const std::string& type, const std::filesystem::path& path) {
    if (ImGui::BeginDragDropSource()) {
#ifdef VC_PLATFORM_LINUX
        const char* itemPath = path.c_str();
        ImGui::SetDragDropPayload(type.c_str(), itemPath, (strlen(itemPath) + 1) * sizeof(char));
#else
        const wchar_t* itemPath = path.c_str();
        ImGui::SetDragDropPayload(type.c_str(), itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
#endif
        ImGui::EndDragDropSource();
    }
}

ContentBrowserPanel::ContentBrowserPanel() : ImGuiWidget("ContentBrowserPanel") {
    m_assetRoot = std::filesystem::path("assets");
    if (!std::filesystem::exists(m_assetRoot)) {
        VC_WARN("Asset path '{}' does not exist, falling back to current directory", m_assetRoot.string());
        m_assetRoot = std::filesystem::current_path();
    }
    m_currentDirectory = m_assetRoot;

    auto d = AssetsManager::load<Texture>("icons/ContentBrowser/directory.png");
    if (d.first != SUCCESS) {
        VC_ERROR("Can't load directory icon: {}", toString(d.first));
    }
    m_directoryIcon = d.second;

    auto f = AssetsManager::load<Texture>("icons/ContentBrowser/file.png");
    if (f.first != SUCCESS) {
        VC_ERROR("Can't load file icon: {}", toString(f.first));
    }
    m_fileIcon = f.second;

    m_expandedPaths.insert(m_assetRoot);
}

void ContentBrowserPanel::render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::Begin("Content Browser");

    ImGui::BeginGroup();
    {
        if (m_currentDirectory != m_assetRoot) {
            if (ImGui::Button("<-")) {
                m_currentDirectory = m_currentDirectory.parent_path();
                ensureTreeExpanded(m_currentDirectory);
            }
            ImGui::SameLine();
        }

        std::string pathStr = std::filesystem::relative(m_currentDirectory, m_assetRoot).string();
        if (pathStr.empty()) pathStr = "assets";
        ImGui::Text("%s", pathStr.c_str());

        ImGui::SameLine(ImGui::GetWindowWidth() - 250);
        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("##search", "Search...", m_searchFilter, IM_ARRAYSIZE(m_searchFilter));
    }
    ImGui::EndGroup();
    ImGui::Separator();

    ImGui::BeginChild("TreePanel", ImVec2(200, 0), true);
    drawFolderTree();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("ContentPanel", ImVec2(0, 0), true);
    drawContentGrid();
    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar();
}

void ContentBrowserPanel::drawFolderTree() {
    drawTreeNode(m_assetRoot);
}

void ContentBrowserPanel::drawTreeNode(const std::filesystem::path& path) {
    std::string label = (path == m_assetRoot) ? "assets" : path.filename().string();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;
    if (path == m_assetRoot)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

    bool isExpanded = (path == m_assetRoot) || (m_expandedPaths.find(path) != m_expandedPaths.end());
    if (isExpanded)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

    if (path == m_currentDirectory) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f)); // gold
    }

    bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);

    if (path == m_currentDirectory)
        ImGui::PopStyleColor();

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        m_currentDirectory = path;
        m_expandedPaths.insert(path);
    }

    if (path != m_assetRoot) {
        if (nodeOpen && !isExpanded) {
            m_expandedPaths.insert(path);
        } else if (!nodeOpen && isExpanded) {
            m_expandedPaths.erase(path);
        }
    }

    if (nodeOpen) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                drawTreeNode(entry.path());
            }
        }
        ImGui::TreePop();
    }
}

void ContentBrowserPanel::ensureTreeExpanded(const std::filesystem::path& path) {
    std::filesystem::path p = path;
    while (p != m_assetRoot && p.has_parent_path()) {
        m_expandedPaths.insert(p);
        p = p.parent_path();
    }
    m_expandedPaths.insert(m_assetRoot);
}

void ContentBrowserPanel::drawContentGrid() {
    float cellWidth  = m_thumbnailSize + m_padding;
    float cellHeight = m_thumbnailSize + ImGui::GetTextLineHeightWithSpacing() + m_padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = (int)(panelWidth / cellWidth);
    if (columns < 1) columns = 1;

    ImGui::Columns(columns, nullptr, false);

    std::string filter = m_searchFilter;
    std::ranges::transform(filter, filter.begin(), ::tolower);

    for (auto& dirEntry : std::filesystem::directory_iterator(m_currentDirectory)) {
        const auto& path = dirEntry.path();
        auto relativePath = std::filesystem::relative(path, m_assetRoot);
        std::string filename = relativePath.filename().string();

        std::string filenameLower = filename;
        std::transform(filenameLower.begin(), filenameLower.end(), filenameLower.begin(), ::tolower);
        if (!filter.empty() && filenameLower.find(filter) == std::string::npos)
            continue;

        ImGui::PushID(filename.c_str());

        // Determine the icon
        std::shared_ptr<Texture> icon;
        if (dirEntry.is_directory()) {
            icon = m_directoryIcon;
        } else {
            AssetType type = AssetsManager::getAssetType(path);
            if (type == AssetType::TEXTURE) {
                auto t = AssetsManager::load<Texture>(relativePath.string());
                if (t.first == SUCCESS) {
                    icon = t.second;
                } else {
                    icon = m_fileIcon; // fallback
                }
            } else {
                icon = m_fileIcon;
            }
        }

        ImVec2 selectableSize(cellWidth - m_padding, cellHeight - m_padding);
        bool selected = false;
        bool clicked = ImGui::Selectable("##item", &selected,  ImGuiSelectableFlags_AllowDoubleClick, selectableSize);

        // Double‑click handling (folders navigate, files can be opened later)
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (dirEntry.is_directory()) {
                m_currentDirectory = path;
                ensureTreeExpanded(path);
            } else {
                // TODO: Open file in editor, if needed
            }
        }

        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemMax = ImGui::GetItemRectMax();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        float thumbPadding = 4.0f;
        ImVec2 thumbMin(itemMin.x + thumbPadding, itemMin.y + thumbPadding);
        ImVec2 thumbMax(itemMin.x + m_thumbnailSize - thumbPadding, itemMin.y + m_thumbnailSize - thumbPadding);

        drawList->AddRectFilled(thumbMin, thumbMax, IM_COL32(30, 30, 30, 200), 4.0f);
        drawList->AddImageRounded((ImTextureID)(intptr_t)icon->getImGuiTextureID(),thumbMin, thumbMax,ImVec2(1, 0), ImVec2(0, 1),IM_COL32_WHITE, 4.0f);

        ImVec2 textPos(itemMin.x, itemMin.y + m_thumbnailSize + 2.0f);
        drawList->AddText(textPos, IM_COL32_WHITE, filename.c_str());

        if (dirEntry.is_directory()) {
            beginDragDropSource("CONTENT_BROWSER_FOLDER", relativePath);
        } else {
            AssetType type = AssetsManager::getAssetType(path);
            switch (type) {
                case TEXTURE: beginDragDropSource("CONTENT_BROWSER_TEXTURE", relativePath); break;
                case SCENE:   beginDragDropSource("CONTENT_BROWSER_SCENE",   relativePath); break;
                case SHADER:  beginDragDropSource("CONTENT_BROWSER_SHADER",  relativePath); break;
                case MESH:    beginDragDropSource("CONTENT_BROWSER_MESH",    relativePath); break;
                default:      beginDragDropSource("CONTENT_BROWSER_FILE",    relativePath); break;
            }
        }

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);
}

} // namespace Vectrix