#include "Editor.h"

#include "Vectrix/EntryPoint.h"
#include "Vectrix/Utils/Path.h"

namespace Vectrix {

    VectrixEditor::VectrixEditor(const std::filesystem::path& launchFile) {
        //PushLayer<EditorLayer>();
        PushLayer(std::make_shared<StartupLayer>(launchFile));
    }

    Application* createApplication(int argc, char** argv) {
        // argv[1] is the file the OS was told to open us
        // on a .vcproj/.vctx file association. UTF-8 on every platform (see getCommandLineArguments)
        const std::filesystem::path launchFile = argc > 1 ? fromUtf8(argv[1]) : std::filesystem::path{};
        return new VectrixEditor(launchFile);
    }
}


VC_SET_APP_INFO("VectrixEditor",0,4,1);