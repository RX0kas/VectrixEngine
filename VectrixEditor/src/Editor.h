#ifndef VECTRIXWORKSPACE_EDITOR_H
#define VECTRIXWORKSPACE_EDITOR_H

#include <Vectrix.h>
#include "Layers/EditorLayer.h"

namespace Vectrix {
	class VectrixEditor : public Application {
	public:
		VectrixEditor();
		~VectrixEditor() override = default;

		static VectrixEditor& instance() {
			VC_CORE_ASSERT(s_instance, "VectrixEditor has not been created");
			return *s_instance;
		}

		static EditorLayer& getEditorLayer() {
			VC_CORE_ASSERT(s_instance, "VectrixEditor has not been created");
			return *s_instance->m_editorLayer;
		}

	private:
		std::shared_ptr<EditorLayer> m_editorLayer;
		static VectrixEditor* s_instance;
	};
}

#endif //VECTRIXWORKSPACE_EDITOR_H
