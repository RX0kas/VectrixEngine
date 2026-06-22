#ifndef VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#define VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H
#include <filesystem>

namespace Vectrix {
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void onImGuiRender();
    private:
        std::filesystem::path m_currentDirectory;
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_CONTENTBROWSERPANEL_H