#include "Editor.h"

#include "Vectrix/EntryPoint.h"

namespace Vectrix {

    VectrixEditor::VectrixEditor() {
        //PushLayer<EditorLayer>();
        PushLayer<StartupLayer>();
    }

    Application* createApplication() {
        return new VectrixEditor();
    }
}


VC_SET_APP_INFO("VectrixEditor",0,2,0);