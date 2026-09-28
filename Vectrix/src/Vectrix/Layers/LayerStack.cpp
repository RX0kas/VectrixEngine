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

	std::shared_ptr<Layer> LayerStack::PopLayer(const std::string& layer) {
		const auto it = m_layers.find(layer);
		if (it == m_layers.end())
			return nullptr;
		std::shared_ptr<Layer> removed = std::move(it->second);
		m_layers.erase(it);
		return removed;
	}

	void LayerStack::destroy() {
		m_layers.clear();
		m_overlays.clear();
	}
}
