#ifndef VECTRIXWORKSPACE_EDITOR_H
#define VECTRIXWORKSPACE_EDITOR_H

#include <Vectrix.h>
#include "Layers/EditorLayer.h"
#include "Layers/StartupLayer.h"

namespace Vectrix {
	class VectrixEditor : public Application {
	public:
		VectrixEditor();
		~VectrixEditor() override = default;
	private:
	};
}

#endif //VECTRIXWORKSPACE_EDITOR_H
