#pragma once

#include "Vectrix/Core.h"
#include "Vectrix/Events/Event.h"

namespace Vectrix {
	class Vectrix_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		[[nodiscard]] const std::string& getName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}