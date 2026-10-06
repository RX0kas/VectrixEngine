#pragma once

#include "Vectrix/Core/DeltaTime.h"
#include "Vectrix/Events/EventListener.h"
#include "Vectrix/Utils/Json.h"

/**
 * @file Layer.h
 * @brief Definition of the Layer interface
 * @ingroup rendering
 */

namespace Vectrix {
	/**
	 * @brief One slice of the application, receiving the update and render callbacks, and the events it subscribes to
	 *
	 * Derive from it and override only what is needed, every callback has an empty default.
	 * The layers are held by a LayerStack, which decides in what order they are called. A layer is an
	 * EventListener: it reacts to events by subscribing to them (see EventListener::subscribe), and only
	 * receives them while it is in the stack.
	 * @see LayerStack
	 * @see Application::pushLayer
	 * @ingroup layers
	 */
	class Layer : public EventListener {
	public:
		/**
		 * @brief Layer constructor
		 * @param name The name of the layer
		 */
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		/**
		 * @brief The function called when the Layer is added to the layer stack
		 */
		virtual void OnAttach() {}


		virtual void OnAttach(const JsonObject& data) { OnAttach(); }

		/**
		 * @brief The function called when the layer is removed from the layer stack
		 */
		virtual void OnDetach() {}

		/**
		 * @brief The function called every update
		 * @param deltaTime The time since the last update
		 */
		virtual void OnUpdate(const DeltaTime& deltaTime) {}

		/**
		 * @brief The function called every frame
		 * @note Use Layer::OnRenderOffscreen if you want to render to a framebuffer for example
		 */
		virtual void OnRender() {}

		/**
		 * @brief The function called every frame before Layer::OnRender
		 */
		virtual void OnRenderOffscreen() {}

		/**
		 * @brief The function called every time we draw all the ImGui widget
		 */
		virtual void OnImGuiRender() {}

		/**
		 * @brief This function return the name of the Layer
		 */
		[[nodiscard]] const std::string& getName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}