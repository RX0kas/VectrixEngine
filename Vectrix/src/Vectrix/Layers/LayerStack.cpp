#include "vcpch.h"
#include "LayerStack.h"

namespace Vectrix {
	LayerStack::LayerStack() {

	}

	LayerStack::~LayerStack() {

	}

	void LayerStack::PushLayer(std::shared_ptr<Layer> layer)
	{
		m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
		m_layerInsertIndex++;
	}

	void LayerStack::PushOverlay(std::shared_ptr<Layer> overlay)
	{
		m_layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(std::shared_ptr<Layer> layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end())
		{
			m_layers.erase(it);
			m_layerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(std::shared_ptr<Layer> overlay)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), overlay);
		if (it != m_layers.end())
			m_layers.erase(it);
	}

	void LayerStack::destroy() {
		m_layers.clear();
		m_layers.shrink_to_fit();
	}
}
