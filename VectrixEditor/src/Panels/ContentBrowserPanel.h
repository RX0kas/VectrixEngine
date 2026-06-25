#ifndef VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#define VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#include <filesystem>

#include "Vectrix/Rendering/Textures/Texture.h"

namespace Vectrix {
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void onImGuiRender();
    private:
        std::filesystem::path m_currentDirectory;
        std::shared_ptr<Texture> m_directoryIcon;
        std::shared_ptr<Texture> m_fileIcon;

        void drawFile(const std::filesystem::path& path, const std::filesystem::path &relativePath);
        void drawFolder(const std::filesystem::path& path, const std::filesystem::path &relativePath);
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H