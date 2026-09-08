#pragma once

#include "Vectrix/Core/Core.h"
#include "Layer.h"

/**
 * @file LayerStack.h
 * @brief Definition of the LayerStack class
 * @ingroup layers
 */

namespace Vectrix {
	/**
	 * @brief Holds the layers of the application, in the order they are processed
	 *
	 * The stack is split in two: the layers, kept at the front, and the overlays, kept at
	 * the back. Updates and renders walk the stack from the first layer to the last
	 * overlay, so an overlay always draws on top. Events go the other way, from the last
	 * overlay down to the first layer, which lets an overlay such as the ImGuiLayer
	 * consume an event before the rest of the application sees it.
	 * @see Layer
	 * @see Application::pushLayer
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
		void PushLayer(std::shared_ptr<Layer> layer);

		/**
		 * @brief Add an overlay, on top of everything else
		 * @param overlay The overlay to add, the stack shares its ownership
		 * @see PushLayer
		 */
		void PushOverlay(std::shared_ptr<Layer> overlay);

		/**
		 * @brief Take a layer out of the stack
		 * @param layer The layer to remove
		 * @note Does nothing when the layer is not in the stack
		 */
		void PopLayer(std::shared_ptr<Layer> layer);

		/**
		 * @brief Take an overlay out of the stack
		 * @param overlay The overlay to remove
		 * @note Does nothing when the overlay is not in the stack
		 */
		void PopOverlay(std::shared_ptr<Layer> overlay);

		/**
		 * @brief Return an iterator on the first layer, so the stack can be walked
		 * @return An iterator to the beginning of the stack
		 */
		std::vector<std::shared_ptr<Layer>>::iterator begin() { return m_layers.begin(); }

		/**
		 * @brief Return an iterator past the last overlay
		 * @return An iterator to the end of the stack
		 */
		std::vector<std::shared_ptr<Layer>>::iterator end() { return m_layers.end(); }

		/**
		 * @brief Detach and drop every layer of the stack
		 * @post The stack is empty
		 */
		void destroy();
	private:
		std::vector<std::shared_ptr<Layer>> m_layers;
		unsigned int m_layerInsertIndex = 0;
	};
}
