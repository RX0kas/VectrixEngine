#include "ContentBrowserPanel.h"

#include <algorithm>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"   // for ImGui::GetWindowDrawList()
#include "Vectrix/Assets/AssetsManager.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    constexpr auto colorSurface = ImVec4(0.105f, 0.110f, 0.125f, 1.00f);
    constexpr auto colorCard = ImVec4(0.130f, 0.135f, 0.150f, 1.00f);
    constexpr auto colorCardHovered = ImVec4(0.180f, 0.185f, 0.200f, 1.00f);
    constexpr auto colorCardSelected = ImVec4(0.170f, 0.200f, 0.260f, 1.00f);
    constexpr auto colorThumbBg = ImVec4(0.200f, 0.205f, 0.230f, 1.00f);
    constexpr auto colorAccent = ImVec4(0.450f, 0.620f, 0.900f, 1.00f);
    constexpr auto colorOutline = ImVec4(0.280f, 0.290f, 0.320f, 1.00f);
    constexpr auto colorTextPrimary = ImVec4(0.950f, 0.950f, 0.980f, 1.00f);
    constexpr auto colorTextMuted = ImVec4(0.550f, 0.570f, 0.620f, 1.00f);
    constexpr auto colorTextDisabled = ImVec4(0.400f, 0.410f, 0.450f, 1.00f);
    constexpr auto colorDanger = ImVec4(0.950f, 0.450f, 0.450f, 1.00f);

    constexpr float cardRounding  = 8.0f;
    constexpr float thumbRounding = 6.0f;

    static const char* s_payloadTypes[] = {
        "CONTENT_BROWSER_FOLDER",
        "CONTENT_BROWSER_TEXTURE",
        "CONTENT_BROWSER_SCENE",
        "CONTENT_BROWSER_SHADER",
        "CONTENT_BROWSER_MESH",
        "CONTENT_BROWSER_FILE",
    };

    static const char* payloadTypeFor(AssetType type) {
        switch (type) {
            case TEXTURE: return "CONTENT_BROWSER_TEXTURE";
            case SCENE:   return "CONTENT_BROWSER_SCENE";
            case SHADER:  return "CONTENT_BROWSER_SHADER";
            case MESH:    return "CONTENT_BROWSER_MESH";
            default:      return "CONTENT_BROWSER_FILE";
        }
    }

    static ImVec4 accentFor(AssetType type) {
        switch (type) {
            case TEXTURE: return {0.42f, 0.72f, 0.52f, 1.0f};
            case SCENE:   return {0.85f, 0.62f, 0.32f, 1.0f};
            case SHADER:  return {0.72f, 0.48f, 0.85f, 1.0f};
            case MESH:    return {0.38f, 0.66f, 0.90f, 1.0f};
            default:      return {0.50f, 0.52f, 0.58f, 1.0f};
        }
    }

    static void beginDragDropSource(const std::string& type, const std::filesystem::path& path) {
        if (ImGui::BeginDragDropSource()) {
#ifdef VC_PLATFORM_LINUX
            const char* itemPath = path.c_str();
            ImGui::SetDragDropPayload(type.c_str(), itemPath, (strlen(itemPath) + 1) * sizeof(char));
#else
            const wchar_t* itemPath = path.c_str();
            ImGui::SetDragDropPayload(type.c_str(), itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
#endif
            ImGui::TextColored(colorTextPrimary, "%s", path.filename().string().c_str());
            ImGui::EndDragDropSource();
        }
    }

    // Rebuilds the asset-relative path carried by a content browser payload.
    static std::filesystem::path payloadToRelativePath(const ImGuiPayload* payload) {
#ifdef VC_PLATFORM_LINUX
        return {static_cast<const char*>(payload->Data)};
#else
        return {static_cast<const wchar_t*>(payload->Data)};
#endif
    }

    static bool isSubPathOf(const std::filesystem::path& parent, const std::filesystem::path& child) {
        auto p = parent.begin();
        auto c = child.begin();
        for (; p != parent.end() && c != child.end(); ++p, ++c) {
            if (*p != *c) return false;
        }
        return p == parent.end();
    }

    static std::string ellipsize(const std::string& text, float maxWidth) {
        if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
            return text;

        std::string out = text;
        while (!out.empty() && ImGui::CalcTextSize((out + "...").c_str()).x > maxWidth)
            out.pop_back();
        return out + "...";
    }

    static std::string toLower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        return text;
    }

    static std::vector<std::filesystem::directory_entry> sortedEntries(const std::filesystem::path& directory) {
        std::vector<std::filesystem::directory_entry> entries;
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(directory, ec))
            entries.push_back(entry);

        if (ec)
            VC_WARN("Can't list '{}': {}", directory.string(), ec.message());

        std::sort(entries.begin(), entries.end(),
            [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) {
                if (a.is_directory() != b.is_directory())
                    return a.is_directory();
                return toLower(a.path().filename().string()) < toLower(b.path().filename().string());
            });

        return entries;
    }

    // Context menu entry that stays visible when unavailable: greyed out, non interactive,
    // and explaining on hover why it can't be used right now.
    static bool contextMenuItem(const char* label, const char* shortcut, bool enabled,
                                const char* disabledHint, const ImVec4& enabledColor = colorTextPrimary) {
        ImGui::PushStyleColor(ImGuiCol_Text, enabled ? enabledColor : colorTextDisabled);
        ImGui::BeginDisabled(!enabled);
        const bool activated = ImGui::MenuItem(label, shortcut);
        ImGui::EndDisabled();
        ImGui::PopStyleColor();

        if (!enabled && disabledHint && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", disabledHint);

        return activated;
    }

    static std::filesystem::path uniqueDestination(const std::filesystem::path& targetDirectory,
                                                   const std::filesystem::path& source) {
        std::filesystem::path candidate = targetDirectory / source.filename();
        if (!std::filesystem::exists(candidate))
            return candidate;

        const std::string stem = source.stem().string();
        const std::string extension = source.extension().string();
        for (int i = 1; i < 1000; ++i) {
            candidate = targetDirectory / (stem + " (" + std::to_string(i) + ")" + extension);
            if (!std::filesystem::exists(candidate))
                return candidate;
        }
        return candidate;
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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, colorSurface);
        ImGui::Begin("Content Browser");

        drawToolbar();

        const float treeWidth = 210.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, cardRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, colorCard);
        ImGui::PushStyleColor(ImGuiCol_Border, colorOutline);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGui::BeginChild("TreePanel", ImVec2(treeWidth, 0), true);
        drawFolderTree();
        ImGui::EndChild();

        ImGui::SameLine(0.0f, 8.0f);

        ImGui::BeginChild("ContentPanel", ImVec2(0, 0), true);
        drawContentGrid();
        ImGui::EndChild();
        ImGui::PopStyleVar(); // WindowPadding

        acceptMoveDropTarget(m_currentDirectory);

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);

        handleShortcuts();
        drawDeletePopup();

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        flushPendingMove();
        flushPendingPaste();
    }

    void ContentBrowserPanel::drawToolbar() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const float height = 36.0f;
        const ImVec2 start = ImGui::GetCursorScreenPos();
        const ImVec2 end(start.x + ImGui::GetContentRegionAvail().x, start.y + height);

        drawList->AddRectFilled(start, end, ImGui::GetColorU32(colorCard), cardRounding);
        drawList->AddRect(start, end, ImGui::GetColorU32(colorOutline), cardRounding);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorCardHovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorThumbBg);

        ImGui::SetCursorScreenPos(ImVec2(start.x + 8.0f, start.y + 6.0f));

        // Back
        const bool canGoUp = (m_currentDirectory != m_assetRoot);
        ImGui::BeginDisabled(!canGoUp);
        if (ImGui::Button("<", ImVec2(24.0f, 24.0f)) && canGoUp) {
            m_currentDirectory = m_currentDirectory.parent_path();
            ensureTreeExpanded(m_currentDirectory);
        }
        ImGui::EndDisabled();

        std::vector<std::filesystem::path> crumbs;
        for (std::filesystem::path p = m_currentDirectory; ; p = p.parent_path()) {
            crumbs.push_back(p);
            if (p == m_assetRoot || !p.has_parent_path())
                break;
        }
        std::reverse(crumbs.begin(), crumbs.end());

        for (size_t i = 0; i < crumbs.size(); ++i) {
            const std::filesystem::path& crumb = crumbs[i];
            const bool isLast = (i + 1 == crumbs.size());
            const std::string label = (crumb == m_assetRoot) ? "assets" : crumb.filename().string();

            ImGui::SameLine(0.0f, 2.0f);
            ImGui::PushID(static_cast<int>(i));
            ImGui::PushStyleColor(ImGuiCol_Text, isLast ? colorTextPrimary : colorTextMuted);
            if (ImGui::Button(label.c_str())) {
                m_currentDirectory = crumb;
                ensureTreeExpanded(crumb);
            }
            ImGui::PopStyleColor();
            acceptMoveDropTarget(crumb);
            ImGui::PopID();

            if (!isLast) {
                ImGui::SameLine(0.0f, 2.0f);
                ImGui::TextColored(colorTextMuted, "/");
            }
        }

        ImGui::PopStyleColor(3);

        const float searchWidth = 200.0f;
        const float sizeWidth   = 90.0f;
        const float rightBlock  = searchWidth + sizeWidth + 16.0f;

        ImGui::SetCursorScreenPos(ImVec2(end.x - rightBlock, start.y + 6.0f));
        ImGui::SetNextItemWidth(sizeWidth);
        ImGui::SliderFloat("##thumbsize", &m_thumbnailSize, 48.0f, 160.0f, "%.0f px");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Thumbnail size");

        ImGui::SameLine(0.0f, 8.0f);
        ImGui::SetNextItemWidth(searchWidth);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colorThumbBg);
        ImGui::InputTextWithHint("##search", "Search...", m_searchFilter, IM_ARRAYSIZE(m_searchFilter));
        ImGui::PopStyleColor();

        ImGui::PopStyleVar(); // FrameRounding

        ImGui::SetCursorScreenPos(ImVec2(start.x, end.y + 8.0f));
    }

    bool ContentBrowserPanel::acceptMoveDropTarget(const std::filesystem::path& targetDirectory) {
        if (!ImGui::BeginDragDropTarget())
            return false;

        for (const char* type : s_payloadTypes) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type);
            if (!payload)
                continue;

            std::filesystem::path source = m_assetRoot / payloadToRelativePath(payload);
            if (source.parent_path() == targetDirectory)
                break;

            if (std::filesystem::is_directory(source) && isSubPathOf(source, targetDirectory)) {
                VC_WARN("Can't move '{}' inside itself", source.string());
                break;
            }

            m_pendingMoveSource = source;
            m_pendingMoveTarget = targetDirectory;
            break;
        }

        ImGui::EndDragDropTarget();
        return true;
    }

    void ContentBrowserPanel::drawContextMenu(const std::filesystem::path& path) {
        const bool isItem = !path.empty();

        // Right-clicking the background still acts on whatever is selected, like the shortcuts do.
        const std::filesystem::path subject = isItem ? path : m_selectedPath;
        const bool hasSubject = !subject.empty() && std::filesystem::exists(subject);

        const char* noSubjectHint = "Select a file or folder first";

        if (contextMenuItem("Copy", "Ctrl+C", hasSubject, noSubjectHint)) {
            m_clipboardPath = subject;
            m_clipboardIsCut = false;
            m_selectedPath = subject;
        }
        if (contextMenuItem("Cut", "Ctrl+X", hasSubject, noSubjectHint)) {
            m_clipboardPath = subject;
            m_clipboardIsCut = true;
            m_selectedPath = subject;
        }

        const std::filesystem::path target = (isItem && std::filesystem::is_directory(path)) ? path : m_currentDirectory;

        const bool canPaste = !m_clipboardPath.empty() && std::filesystem::exists(m_clipboardPath);
        if (contextMenuItem("Paste", "Ctrl+V", canPaste, "Nothing has been copied or cut yet"))
            pasteInto(target);

        ImGui::Separator();

        if (contextMenuItem("Delete", "Del", hasSubject, noSubjectHint, colorDanger)) {
            m_pendingDeletePath = subject;
            m_openDeletePopup = true;
        }

        // Make it obvious which entry the actions apply to when it isn't the one under the cursor.
        if (hasSubject && !isItem) {
            ImGui::Separator();
            ImGui::TextColored(colorTextMuted, "Selected: %s", subject.filename().string().c_str());
        }

        if (canPaste) {
            ImGui::Separator();
            ImGui::TextColored(colorTextMuted, "%s: %s", m_clipboardIsCut ? "Cut" : "Copied", m_clipboardPath.filename().string().c_str());
        }
    }

    void ContentBrowserPanel::handleShortcuts() {
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            return;
        if (ImGui::GetIO().WantTextInput)
            return; // don't steal keys from the search field

        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) && !m_selectedPath.empty()) {
            m_pendingDeletePath = m_selectedPath;
            m_openDeletePopup = true;
        }

        if (!ImGui::GetIO().KeyCtrl)
            return;

        if (ImGui::IsKeyPressed(ImGuiKey_C, false) && !m_selectedPath.empty()) {
            m_clipboardPath = m_selectedPath;
            m_clipboardIsCut = false;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_X, false) && !m_selectedPath.empty()) {
            m_clipboardPath = m_selectedPath;
            m_clipboardIsCut = true;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_V, false)) {
            // Pasting with a folder selected drops the copy inside it.
            const bool intoSelection = !m_selectedPath.empty() && std::filesystem::is_directory(m_selectedPath);
            pasteInto(intoSelection ? m_selectedPath : m_currentDirectory);
        }
    }

    void ContentBrowserPanel::pasteInto(const std::filesystem::path& targetDirectory) {
        if (m_clipboardPath.empty())
            return;

        if (!std::filesystem::exists(m_clipboardPath)) {
            VC_WARN("Clipboard entry '{}' no longer exists", m_clipboardPath.string());
            m_clipboardPath.clear();
            return;
        }

        if (std::filesystem::is_directory(m_clipboardPath) && isSubPathOf(m_clipboardPath, targetDirectory)) {
            VC_WARN("Can't paste '{}' inside itself", m_clipboardPath.string());
            return;
        }

        if (m_clipboardIsCut && m_clipboardPath.parent_path() == targetDirectory) {
            m_clipboardPath.clear();
            return;
        }

        m_pendingPasteTarget = targetDirectory;
    }

    void ContentBrowserPanel::flushPendingPaste() {
        if (m_pendingPasteTarget.empty())
            return;

        const std::filesystem::path source = m_clipboardPath;
        const std::filesystem::path target = m_pendingPasteTarget;
        m_pendingPasteTarget.clear();

        const std::filesystem::path destination = uniqueDestination(target, source);
        const bool isCut = m_clipboardIsCut;

        std::error_code ec;
        if (isCut) {
            std::filesystem::rename(source, destination, ec);
        } else if (std::filesystem::is_directory(source)) {
            std::filesystem::copy(source, destination,
                                  std::filesystem::copy_options::recursive, ec);
        } else {
            std::filesystem::copy_file(source, destination, ec);
        }

        if (ec) {
            VC_ERROR("Failed to {} '{}' to '{}': {}", isCut ? "move" : "copy",
                     source.string(), destination.string(), ec.message());
            return;
        }

        if (isCut) {
            m_clipboardPath.clear();
            m_clipboardIsCut = false;

            if (std::filesystem::is_directory(destination)) {
                if (isSubPathOf(source, m_currentDirectory))
                    m_currentDirectory = destination;
                m_expandedPaths.erase(source);
            }
        }

        m_selectedPath = destination;
        if (std::filesystem::is_directory(destination))
            ensureTreeExpanded(destination);
    }

    void ContentBrowserPanel::deleteEntry(const std::filesystem::path& path) {
        if (path.empty() || path == m_assetRoot)
            return;

        std::error_code ec;
        const bool isDirectory = std::filesystem::is_directory(path);
        if (isDirectory)
            std::filesystem::remove_all(path, ec);
        else
            std::filesystem::remove(path, ec);

        if (ec) {
            VC_ERROR("Failed to delete '{}': {}", path.string(), ec.message());
            return;
        }

        VC_INFO("Deleted '{}'", path.string());

        if (m_selectedPath == path)
            m_selectedPath.clear();
        if (m_clipboardPath == path) {
            m_clipboardPath.clear();
            m_clipboardIsCut = false;
        }

        if (isDirectory) {
            m_expandedPaths.erase(path);
            if (isSubPathOf(path, m_currentDirectory))
                m_currentDirectory = path.parent_path();
        }
    }

    void ContentBrowserPanel::drawDeletePopup() {
        if (m_openDeletePopup) {
            ImGui::OpenPopup("Delete asset##ConfirmDelete");
            m_openDeletePopup = false;
        }

        ImGui::SetNextWindowSize(ImVec2(360, 0), ImGuiCond_Appearing);
        if (!ImGui::BeginPopupModal("Delete asset##ConfirmDelete", nullptr, ImGuiWindowFlags_NoResize))
            return;

        const bool isDirectory = std::filesystem::is_directory(m_pendingDeletePath);

        ImGui::TextColored(colorTextPrimary, "Delete %s?", isDirectory ? "this folder" : "this file");
        ImGui::Spacing();
        ImGui::TextWrapped("%s", m_pendingDeletePath.filename().string().c_str());
        ImGui::Spacing();

        if (isDirectory)
            ImGui::TextColored(ImVec4(0.95f, 0.60f, 0.35f, 1.0f), "Everything inside will be deleted too.");
        ImGui::TextColored(colorTextMuted, "This can't be undone.");

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_pendingDeletePath.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0f, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.16f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.22f, 0.22f, 1.0f));
        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            deleteEntry(m_pendingDeletePath);
            m_pendingDeletePath.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::EndPopup();
    }

    void ContentBrowserPanel::flushPendingMove() {
        if (m_pendingMoveSource.empty())
            return;

        std::filesystem::path source = m_pendingMoveSource;
        std::filesystem::path destination = m_pendingMoveTarget / source.filename();
        m_pendingMoveSource.clear();
        m_pendingMoveTarget.clear();

        if (std::filesystem::exists(destination)) {
            VC_WARN("'{}' already exists, move cancelled", destination.string());
            return;
        }

        std::error_code ec;
        std::filesystem::rename(source, destination, ec);
        if (ec) {
            VC_ERROR("Failed to move '{}' to '{}': {}", source.string(), destination.string(), ec.message());
            return;
        }

        if (m_selectedPath == source)
            m_selectedPath = destination;

        if (std::filesystem::is_directory(destination)) {
            if (isSubPathOf(source, m_currentDirectory))
                m_currentDirectory = destination;
            m_expandedPaths.erase(source);
            ensureTreeExpanded(destination);
        }
    }

    void ContentBrowserPanel::drawFolderTree() {
        ImGui::PushStyleColor(ImGuiCol_Header, colorCardSelected);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, colorCardHovered);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, colorCardSelected);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 3));

        drawTreeNode(m_assetRoot);

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
    }

    void ContentBrowserPanel::drawTreeNode(const std::filesystem::path& path) {
        std::string label = (path == m_assetRoot) ? "assets" : path.filename().string();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;
        if (path == m_assetRoot)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;

        bool isExpanded = (path == m_assetRoot) || (m_expandedPaths.find(path) != m_expandedPaths.end());
        if (isExpanded)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;

        if (path == m_selectedPath)
            flags |= ImGuiTreeNodeFlags_Selected;

        const bool isCurrent = (path == m_currentDirectory);
        ImGui::PushStyleColor(ImGuiCol_Text, isCurrent ? colorAccent : colorTextPrimary);
        bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);
        ImGui::PopStyleColor();

        // Folders in the tree are both draggable and drop targets.
        if (path != m_assetRoot)
            beginDragDropSource("CONTENT_BROWSER_FOLDER", std::filesystem::relative(path, m_assetRoot));

        if (acceptMoveDropTarget(path)) {
            ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                                                ImGui::GetColorU32(colorAccent), 4.0f, 0, 2.0f);
        }

        if (ImGui::BeginPopupContextItem(label.c_str())) {
            m_selectedPath = path;
            drawContextMenu(path);
            ImGui::EndPopup();
        }

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_currentDirectory = path;
            m_selectedPath = path;
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
            for (const auto& entry : sortedEntries(path)) {
                if (entry.is_directory())
                    drawTreeNode(entry.path());
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
        const float labelHeight = ImGui::GetTextLineHeight() + 10.0f;
        const ImVec2 cellSize(m_thumbnailSize, m_thumbnailSize + labelHeight);

        const float panelWidth = ImGui::GetContentRegionAvail().x;
        int columns = static_cast<int>((panelWidth + m_padding) / (cellSize.x + m_padding));
        if (columns < 1) columns = 1;

        const std::string filter = toLower(m_searchFilter);

        int drawn = 0;
        for (const auto& entry : sortedEntries(m_currentDirectory)) {
            const std::string filename = entry.path().filename().string();
            if (!filter.empty() && toLower(filename).find(filter) == std::string::npos)
                continue;

            if (drawn % columns != 0)
                ImGui::SameLine(0.0f, m_padding);

            drawGridItem(entry, cellSize);
            ++drawn;
        }

        if (ImGui::BeginPopupContextWindow("##GridContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            drawContextMenu({});
            ImGui::EndPopup();
        }

        if (drawn == 0) {
            ImGui::Dummy(ImVec2(0, 8));
            ImGui::TextColored(colorTextMuted, filter.empty() ? "  This folder is empty" : "  No asset matches the search");
        }

        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_selectedPath.clear();
    }

    void ContentBrowserPanel::drawGridItem(const std::filesystem::directory_entry& entry, const ImVec2& cellSize) {
        const std::filesystem::path& path = entry.path();
        const std::filesystem::path relativePath = std::filesystem::relative(path, m_assetRoot);
        const std::string filename = path.filename().string();
        const bool isDirectory = entry.is_directory();
        const AssetType type = isDirectory ? AssetType::UNKNOWN : AssetsManager::getAssetType(path);

        ImGui::PushID(filename.c_str());

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 cardMin = ImGui::GetCursorScreenPos();
        const ImVec2 cardMax(cardMin.x + cellSize.x, cardMin.y + cellSize.y);

        ImGui::InvisibleButton("##tile", cellSize);

        const bool hovered  = ImGui::IsItemHovered();
        const bool selected = (path == m_selectedPath);

        if (ImGui::IsItemClicked())
            m_selectedPath = path;

        if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (isDirectory) {
                m_currentDirectory = path;
                ensureTreeExpanded(path);
            } else {
                handleDoubleClick(path);
            }
        }

        if (ImGui::BeginPopupContextItem("##ItemContext")) {
            m_selectedPath = path;
            drawContextMenu(path);
            ImGui::EndPopup();
        }

        beginDragDropSource(isDirectory ? "CONTENT_BROWSER_FOLDER" : payloadTypeFor(type), relativePath);

        const bool dropHovered = isDirectory && acceptMoveDropTarget(path);

        ImVec4 background = colorCard;
        if (selected) background = colorCardSelected;
        else if (hovered) background = colorCardHovered;

        drawList->AddRectFilled(cardMin, cardMax, ImGui::GetColorU32(background), cardRounding);

        if (selected || dropHovered)
            drawList->AddRect(cardMin, cardMax, ImGui::GetColorU32(colorAccent), cardRounding, 0, 2.0f);
        else if (hovered)
            drawList->AddRect(cardMin, cardMax, ImGui::GetColorU32(colorOutline), cardRounding, 0, 1.0f);

        const float inset = 8.0f;
        const float thumbSize = cellSize.x - inset * 2.0f;
        const ImVec2 thumbMin(cardMin.x + inset, cardMin.y + inset);
        const ImVec2 thumbMax(thumbMin.x + thumbSize, thumbMin.y + thumbSize);

        drawList->AddRectFilled(thumbMin, thumbMax, ImGui::GetColorU32(colorThumbBg), thumbRounding);

        std::shared_ptr<Texture> icon = isDirectory ? m_directoryIcon : m_fileIcon;
        if (type == AssetType::TEXTURE) {
            auto t = AssetsManager::load<Texture>(relativePath.string());
            if (t.first == SUCCESS)
                icon = t.second;
        }

        const bool isCutSource = m_clipboardIsCut && path == m_clipboardPath;
        const ImU32 thumbTint = isCutSource ? IM_COL32(255, 255, 255, 110) : IM_COL32_WHITE;

        if (icon) {
            drawList->AddImageRounded(static_cast<ImTextureID>(static_cast<intptr_t>(icon->getImGuiTextureID())),
                                      thumbMin, thumbMax, ImVec2(1, 0), ImVec2(0, 1),
                                      thumbTint, thumbRounding);
        }

        // type badge
        if (!isDirectory) {
            std::string ext = path.extension().string();
            if (!ext.empty()) {
                ext = toLower(ext.substr(1));
                const ImVec2 textSize = ImGui::CalcTextSize(ext.c_str());
                const ImVec2 badgeMax(thumbMax.x - 4.0f, thumbMax.y - 4.0f);
                const ImVec2 badgeMin(badgeMax.x - textSize.x - 8.0f, badgeMax.y - textSize.y - 2.0f);

                drawList->AddRectFilled(badgeMin, badgeMax, ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.06f, 0.85f)), 4.0f);
                drawList->AddText(ImVec2(badgeMin.x + 4.0f, badgeMin.y + 1.0f),
                                  ImGui::GetColorU32(accentFor(type)), ext.c_str());
            }
        }

        const std::string displayName = isDirectory ? filename : path.stem().string();

        const float labelWidth = cellSize.x - 8.0f;
        const std::string label = ellipsize(displayName, labelWidth);
        const ImVec2 labelSize = ImGui::CalcTextSize(label.c_str());

        ImVec4 labelColor = selected ? colorTextPrimary : ImVec4(0.78f, 0.80f, 0.86f, 1.0f);
        if (isCutSource)
            labelColor.w = 0.45f;

        drawList->AddText(ImVec2(cardMin.x + (cellSize.x - labelSize.x) * 0.5f, thumbMax.y + 5.0f),
                          ImGui::GetColorU32(labelColor), label.c_str());

        if (hovered && label != displayName)
            ImGui::SetTooltip("%s", filename.c_str());

        ImGui::PopID();
    }

    void ContentBrowserPanel::handleDoubleClick(std::filesystem::path path) {

    }
} // namespace Vectrix