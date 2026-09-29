#include "vcpch.h"
#include "LayerStack.h"

#include <algorithm>

namespace Vectrix {
	LayerStack::LayerStack() = default;

	LayerStack::~LayerStack() = default;

	bool LayerStack::contains(const Layer* layer) const {
		const auto same = [layer](const std::shared_ptr<Layer>& other) { return other.get() == layer; };
		return std::ranges::any_of(m_layers, same) || std::ranges::any_of(m_overlays, same);
	}

	void LayerStack::PushLayer(const std::shared_ptr<Layer>& layer) {
		if (contains(layer.get())) {
			VC_CORE_WARN("Layer {} is already in the layer stack", layer->getName());
			return;
		}
		m_layers.push_back(layer);
	}

	void LayerStack::PushOverlay(const std::shared_ptr<Layer>& overlay) {
		if (contains(overlay.get())) {
			VC_CORE_WARN("Overlay {} is already in the layer stack", overlay->getName());
			return;
		}
		m_overlays.push_back(overlay);
	}

	std::shared_ptr<Layer> LayerStack::PopLayer(const std::string& layer) {
		for (std::vector<std::shared_ptr<Layer>>* group : {&m_layers, &m_overlays}) {
			const auto it = std::ranges::find_if(*group, [&layer](const std::shared_ptr<Layer>& l) { return l->getName() == layer; });
			if (it == group->end())
				continue;
			std::shared_ptr<Layer> removed = std::move(*it);
			group->erase(it);
			return removed;
		}
		return nullptr;
	}

	std::shared_ptr<Layer> LayerStack::ReplaceLayer(const Layer* oldLayer, std::shared_ptr<Layer> newLayer) {
		for (std::vector<std::shared_ptr<Layer>>* group : {&m_layers, &m_overlays}) {
			const auto it = std::ranges::find_if(*group, [oldLayer](const std::shared_ptr<Layer>& l) { return l.get() == oldLayer; });
			if (it == group->end())
				continue;
			std::shared_ptr<Layer> replaced = std::move(*it);
			*it = std::move(newLayer);
			return replaced;
		}
		m_layers.push_back(std::move(newLayer));
		return nullptr;
	}

	void LayerStack::destroy() {
		m_layers.clear();
		m_overlays.clear();
	}
}
