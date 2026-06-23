#include "ContentBrowserPanel.h"

#include "imgui.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    static const std::filesystem::path s_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel() {
        if (std::filesystem::path(s_AssetPath).empty()) {
            VC_WARN("{} doesn't exist for the asset path",s_AssetPath.string());
            m_currentDirectory = std::filesystem::current_path();
        } else {
            m_currentDirectory = s_AssetPath;
        }
    }

    void ContentBrowserPanel::onImGuiRender() {
        ImGui::Begin("Content Browser");

        if (m_currentDirectory != std::filesystem::path(s_AssetPath))
        {
            if (ImGui::Button("<-"))
            {
                m_currentDirectory = m_currentDirectory.parent_path();
            }
        }

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory))
        {
            const auto& path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, s_AssetPath);
            std::string filenameString = relativePath.filename().string();
            if (directoryEntry.is_directory())
            {
                if (ImGui::Button(filenameString.c_str()))
                {
                    m_currentDirectory /= path.filename();
                }
            }
            else
            {
                if (ImGui::Button(filenameString.c_str()))
                {
                }
            }
        }

        ImGui::End();
    }
} // Vectrix