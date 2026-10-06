#ifndef VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#define VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H

#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "imgui.h"
#include "Vectrix/Events/EventListener.h"
#include "Vectrix/ImGui/ImGuiWidget.h"
#include "Vectrix/Rendering/Textures/Texture.h"

namespace Vectrix {

    class ContentBrowserPanel : public ImGuiWidget {
    public:
        /**
         * @param assetRoot The folder to browse, e.g. the open project's `Assets` folder
         * @param events The listener the panel receives the files dropped onto the window from (its layer)
         * @note Sends an AssetMovedEvent for every file or folder it moves or renames
         */
        ContentBrowserPanel(const std::filesystem::path& assetRoot, EventListener& events);

        void render() override;

        /// Copies files or folders into the current folder, under a free name; used for the files dropped onto
        /// the window. Those already in the current folder are left alone
        void importFiles(const std::vector<std::string>& paths);

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
        /// Updates what points at a moved file or folder (selection, current folder, clipboard), then sends AssetMovedEvent
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

        ScopedSubscription m_onFilesDropped;

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
        std::string m_lastImportErrorMessage;
    };

} // namespace Vectrix

#endif //VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H