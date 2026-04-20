#include <Vectrix.h>
#include "Layers/EditorLayer.h"

#include "Vectrix/EntryPoint.h"

namespace Vectrix {
    class VectrixEditor : public Application {
    public:
        VectrixEditor() {
            m_editorLayer = std::make_shared<EditorLayer>();
            PushLayer(m_editorLayer);
        }
        ~VectrixEditor() override = default;
    private:
        std::shared_ptr<EditorLayer> m_editorLayer;
    };


    Application* createApplication() {
        return new VectrixEditor();
    }
}


VC_SET_APP_INFO("VectrixEditor",0,1,0);