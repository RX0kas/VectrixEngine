#pragma once

#include "../Core/Core.h"
#include "Vectrix/Core/DeltaTime.h"
#include "Vectrix/Events/Event.h"

namespace Vectrix {
	class Vectrix_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(DeltaTime deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		[[nodiscard]] const std::string& getName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}