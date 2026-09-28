#pragma once

#include "Vectrix/Core/Core.h"
#include "Layer.h"
#include "Vectrix/Utils/Memory.h"

/**
 * @file LayerStack.h
 * @brief Definition of the LayerStack class
 * @ingroup layers
 */

namespace Vectrix {
	/**
	 * @brief Holds the layers of the application, keyed by their name
	 *
	 * The stack is split in two groups: the layers and the overlays. Updates and renders
	 * go through every layer then every overlay, so an overlay always draws on top; events
	 * go through the overlays first. Within a group there is no order: the storage is a
	 * hash map, so two layers of the same group must not rely on running one before the
	 * other, and two layers can't share a name.
	 * @see Layer
	 * @see Application::PushLayer
	 * @ingroup layers
	 */
	class LayerStack {
	public:
		LayerStack();
		~LayerStack();

		/**
		 * @brief Add a layer, on top of the layers already there but under the overlays
		 * @param layer The layer to add, the stack shares its ownership
		 * @note Layer::OnAttach is called by the Application, not by the stack
		 */
		void PushLayer(const std::shared_ptr<Layer>& layer);

		/**
		 * @brief Add an overlay, on top of everything else
		 * @param overlay The overlay to add, the stack shares its ownership
		 * @see PushLayer
		 */
		void PushOverlay(const std::shared_ptr<Layer>& overlay);

		/**
		 * @brief Take a layer out of the stack
		 * @param layer The name of the layer to remove
		 * @return The removed layer, or nullptr when no layer has that name
		 * @note Layer::OnDetach is called by the Application, not by the stack
		 */
		std::shared_ptr<Layer> PopLayer(const std::string& layer);

		using iterator = Cache<std::string, std::shared_ptr<Layer>>::iterator;

		iterator beginLayers() { return m_layers.begin(); }
		iterator endLayers() { return m_layers.end(); }

		iterator beginOverlays() { return m_overlays.begin(); }
		iterator endOverlays() { return m_overlays.end(); }

		/**
		 * @brief Drop every layer and overlay of the stack
		 * @post The stack is empty
		 * @note Layer::OnDetach is called by the Application beforehand
		 */
		void destroy();
	private:
		Cache<std::string,std::shared_ptr<Layer>> m_layers;
		Cache<std::string,std::shared_ptr<Layer>> m_overlays;
	};
}
