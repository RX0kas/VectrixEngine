#pragma once

#include "Vectrix/Core/Core.h"
#include "Layer.h"

namespace Vectrix {
	class LayerStack {
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(std::shared_ptr<Layer> layer);
		void PushOverlay(std::shared_ptr<Layer> overlay);
		void PopLayer(std::shared_ptr<Layer> layer);
		void PopOverlay(std::shared_ptr<Layer> overlay);

		std::vector<std::shared_ptr<Layer>>::iterator begin() { return m_layers.begin(); }
		std::vector<std::shared_ptr<Layer>>::iterator end() { return m_layers.end(); }

		void destroy();
	private:
		std::vector<std::shared_ptr<Layer>> m_layers;
		unsigned int m_layerInsertIndex = 0;
	};
}
