#include "vcpch.h"
#include "LayerStack.h"

namespace Vectrix {
	LayerStack::LayerStack() {

	}

	LayerStack::~LayerStack() {

	}

	void LayerStack::PushLayer(const std::shared_ptr<Layer>& layer) {
		m_layers.emplace(layer->getName(), layer);
	}

	void LayerStack::PushOverlay(const std::shared_ptr<Layer>& overlay)
	{
		m_overlays.emplace(overlay->getName(),overlay);
	}

	void LayerStack::PopLayer(const std::shared_ptr<Layer>& layer) {
		m_layers.erase(layer->getName());
	}

	void LayerStack::PopLayer(const std::string& layer) {
		m_layers.erase(layer);
	}

	void LayerStack::PopOverlay(const std::shared_ptr<Layer>& overlay) {
		m_overlays.erase(overlay->getName());
	}

	void LayerStack::destroy() {
		m_layers.clear();
	}
}
