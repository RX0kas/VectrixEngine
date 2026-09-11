#ifndef VECTRIXWORKSPACE_STARTUPLAYER_H
#define VECTRIXWORKSPACE_STARTUPLAYER_H
#include "Vectrix/Layers/Layer.h"

namespace Vectrix {
	class StartupLayer : public Layer {
	public:
		StartupLayer();
		~StartupLayer();
		void OnImGuiRender() override;
		void OnAttach() override;
	private:
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_STARTUPLAYER_H
