#ifndef VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#define VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H

#include <filesystem>
#include <set>
#include <string>

#include "imgui.h"
#include "Vectrix/ImGui/ImGuiWidget.h"
#include "Vectrix/Rendering/Textures/Texture.h"

namespace Vectrix {

    class ContentBrowserPanel : public ImGuiWidget {
    public:
        ContentBrowserPanel();

        void render() override;

    private:
        void drawToolbar();
        void drawFolderTree();
        void drawTreeNode(const std::filesystem::path& path);
        void drawContentGrid();
        void drawGridItem(const std::filesystem::directory_entry& entry, const ImVec2& cellSize);
        void ensureTreeExpanded(const std::filesystem::path& path);

        bool acceptMoveDropTarget(const std::filesystem::path& targetDirectory);
        void flushPendingMove();

        void drawContextMenu(const std::filesystem::path& path);
        void handleShortcuts();
        void pasteInto(const std::filesystem::path& targetDirectory);
        void flushPendingPaste();
        void drawDeletePopup();
        void deleteEntry(const std::filesystem::path& path);
        void handleDoubleClick(std::filesystem::path path);

        std::filesystem::path m_assetRoot;
        std::filesystem::path m_currentDirectory;
        std::filesystem::path m_selectedPath;

        std::filesystem::path m_pendingMoveSource;
        std::filesystem::path m_pendingMoveTarget;

        std::filesystem::path m_clipboardPath;
        std::filesystem::path m_pendingPasteTarget;
        bool m_clipboardIsCut = false;

        std::filesystem::path m_pendingDeletePath;
        bool m_openDeletePopup = false;

        std::shared_ptr<Texture> m_directoryIcon;
        std::shared_ptr<Texture> m_fileIcon;

        std::set<std::filesystem::path> m_expandedPaths;

        char m_searchFilter[256] = "";

        float m_thumbnailSize = 84.0f;
        float m_padding = 12.0f;
    };

} // namespace Vectrix

#endif //VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H