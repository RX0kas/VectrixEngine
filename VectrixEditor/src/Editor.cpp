#include "Editor.h"

#include "Vectrix/EntryPoint.h"

namespace Vectrix {

    VectrixEditor::VectrixEditor() {
        s_instance = this;
        m_editorLayer = std::make_shared<EditorLayer>();
        PushLayer(m_editorLayer);
    }


    VectrixEditor* VectrixEditor::s_instance = nullptr;

    Application* createApplication() {
        return new VectrixEditor();
    }
}


VC_SET_APP_INFO("VectrixEditor",0,2,0);