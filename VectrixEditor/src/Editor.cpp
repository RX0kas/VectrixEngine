#include "Editor.h"

#include "Vectrix/EntryPoint.h"

namespace Vectrix {

    VectrixEditor::VectrixEditor(const std::filesystem::path& launchFile) {
        //PushLayer<EditorLayer>();
        PushLayer(std::make_shared<StartupLayer>(launchFile));
    }

    Application* createApplication(int argc, char** argv) {
        // argv[1] is the file the OS was told to open us
        // on a .vcproj/.vctx file association
        const std::filesystem::path launchFile = argc > 1 ? std::filesystem::path(argv[1]) : std::filesystem::path{};
        return new VectrixEditor(launchFile);
    }
}


VC_SET_APP_INFO("VectrixEditor",0,4,0);