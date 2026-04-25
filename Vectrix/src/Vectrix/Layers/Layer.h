#pragma once

#include "Vectrix/Core/DeltaTime.h"
#include "Vectrix/Events/Event.h"

/**
 * @file Layer.h
 * @brief Definition of the Layer interface
 * @ingroup rendering
 */

namespace Vectrix {
	class Layer {
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
		 * @brief Called every time an event occurre
		 * @param event The event that occurred
		 */
		virtual void OnEvent(Event& event) {}

		/**
		 * @brief This function return the name of the Layer
		 */
		[[nodiscard]] const std::string& getName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}