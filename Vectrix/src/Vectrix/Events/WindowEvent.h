#pragma once
#include "Vectrix/Events/Event.h"

namespace Vectrix {
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
			EVENT_CLASS_CATEGORY(EventCategoryWindow)
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {
		}

		[[nodiscard]] unsigned int getWidth() const { return m_Width; }
		[[nodiscard]] unsigned int getHeight() const { return m_Height; }

		[[nodiscard]] std::string toString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
			EVENT_CLASS_CATEGORY(EventCategoryWindow)
	private:
		unsigned int m_Width, m_Height;
	};
}