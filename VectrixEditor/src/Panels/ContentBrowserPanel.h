#ifndef VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#define VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H

#include <filesystem>
#include <set>
#include <string>

#include "Vectrix/ImGui/ImGuiWidget.h"
#include "Vectrix/Rendering/Textures/Texture.h"

namespace Vectrix {

    class ContentBrowserPanel : public ImGuiWidget {
    public:
        ContentBrowserPanel();

        void render() override;

    private:
        void drawFolderTree();
        void drawTreeNode(const std::filesystem::path& path);
        void drawContentGrid();
        void ensureTreeExpanded(const std::filesystem::path& path);

        std::filesystem::path m_assetRoot;
        std::filesystem::path m_currentDirectory;

        std::shared_ptr<Texture> m_directoryIcon;
        std::shared_ptr<Texture> m_fileIcon;

        std::set<std::filesystem::path> m_expandedPaths;

        char m_searchFilter[256] = "";

        float m_thumbnailSize = 80.0f;
        float m_padding = 8.0f;
    };

} // namespace Vectrix

#endif //VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H