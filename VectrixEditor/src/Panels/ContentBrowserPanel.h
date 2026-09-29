#ifndef VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#define VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H

#include <filesystem>
#include <functional>
#include <set>
#include <string>

#include "imgui.h"
#include "Vectrix/ImGui/ImGuiWidget.h"
#include "Vectrix/Rendering/Textures/Texture.h"

namespace Vectrix {

    class ContentBrowserPanel : public ImGuiWidget {
    public:
        /**
         * @param assetRoot The folder to browse, e.g. the open project's `Assets` folder
         */
        explicit ContentBrowserPanel(const std::filesystem::path& assetRoot);

        void render() override;

        /// Called with the old and new path of a file or folder the panel moved or renamed, once it's done
        using MovedCallback = std::function<void(const std::filesystem::path& from, const std::filesystem::path& to)>;
        /// Set what follows a move (EditorLayer updates the scenes that use the asset)
        void setOnMoved(MovedCallback callback) { m_onMoved = std::move(callback); }

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
        void startRename(const std::filesystem::path& path);
        void drawRenamePopup();
        void renameEntry(const std::filesystem::path& path, const std::string& newName);
        /// Updates what points at a moved file or folder (selection, current folder, clipboard), then calls m_onMoved
        void entryMoved(const std::filesystem::path& from, const std::filesystem::path& to);
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

        std::filesystem::path m_renamePath;
        bool m_openRenamePopup = false;
        char m_renameBuffer[256] = "";

        MovedCallback m_onMoved;

        std::shared_ptr<Texture> m_directoryIcon;
        std::shared_ptr<Texture> m_fileIcon;

        std::set<std::filesystem::path> m_expandedPaths;

        char m_searchFilter[256] = "";

        float m_thumbnailSize = 84.0f;
        float m_padding = 12.0f;

        std::string m_lastDirIconErrorMessage;
        std::string m_lastFileIconErrorMessage;
        std::string m_lastFlushPendingPasteErrorMessage;
        std::string m_lastDeleteErrorMessage;
        std::string m_lastFlushPendingMoveErrorMessage;
        std::string m_lastRenameErrorMessage;
    };

} // namespace Vectrix

#endif //VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H