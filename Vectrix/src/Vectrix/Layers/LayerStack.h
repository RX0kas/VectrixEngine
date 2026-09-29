#pragma once

#include "Vectrix/Core/Core.h"
#include "Layer.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @file LayerStack.h
 * @brief Definition of the LayerStack class
 * @ingroup layers
 */

namespace Vectrix {
	/**
	 * @brief Holds the layers of the application, in the order they were pushed
	 *
	 * The stack is split in two groups: the layers, then the overlays above them. Updates and renders go from the
	 * bottom up (the layers in the order they were pushed, then the overlays), so what is pushed later draws on
	 * top; events go from the top down (the last overlay pushed first, the first layer pushed last), so what is
	 * on top can handle an event before what is under it. Names don't have to be unique.
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
		 * @note Layer::OnAttach is called by the Application, not by the stack. A layer already in the stack isn't
		 *       added again
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
		 * @param layer The name of the layer to remove: the lowest layer with that name, else overlay
		 * @return The removed layer, or nullptr when no layer has that name
		 * @note Layer::OnDetach is called by the Application, not by the stack
		 */
		std::shared_ptr<Layer> PopLayer(const std::string& layer);

		/**
		 * @brief Put a layer in the place of another, so the order of the others doesn't change
		 * @param oldLayer The layer to replace
		 * @param newLayer The layer taking its place (on top of the layers when oldLayer isn't in the stack)
		 * @return The replaced layer, or nullptr when oldLayer isn't in the stack
		 * @note Layer::OnDetach and OnAttach are called by the Application
		 * @see Application::switchToLayer
		 */
		std::shared_ptr<Layer> ReplaceLayer(const Layer* oldLayer, std::shared_ptr<Layer> newLayer);

		/// The layers, from the bottom (pushed first) to the top
		[[nodiscard]] const std::vector<std::shared_ptr<Layer>>& layers() const { return m_layers; }
		/// The overlays, from the bottom (pushed first) to the top
		[[nodiscard]] const std::vector<std::shared_ptr<Layer>>& overlays() const { return m_overlays; }

		/**
		 * @brief Drop every layer and overlay of the stack
		 * @post The stack is empty
		 * @note Layer::OnDetach is called by the Application beforehand
		 */
		void destroy();
	private:
		/// @return true when layer is in the stack already
		[[nodiscard]] bool contains(const Layer* layer) const;

		std::vector<std::shared_ptr<Layer>> m_layers;
		std::vector<std::shared_ptr<Layer>> m_overlays;
	};
}
