#pragma once

#include "vcpch.h"
#include "../Core/Core.h"

namespace Vectrix {
	// TODO : Events should not block the entire app

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryWindow = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

	// A macro to not have to rewrite those tree function every time we create a new event
#define EVENT_CLASS_TYPE(type) \
	static EventType getStaticType() { return EventType::type; } \
	virtual EventType getEventType() const override { return getStaticType(); } \
	virtual const char* getName() const override { return #type; }


//  A macro to not have to rewrite the getCategoryFlags function every time we create a new event
#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }


	class Vectrix_API Event
	{
	public:
		virtual ~Event() = default;
		bool Handled = false;

		[[nodiscard]] virtual EventType getEventType() const = 0;
		[[nodiscard]] virtual const char* getName() const = 0;
		[[nodiscard]] virtual int getCategoryFlags() const = 0;
		[[nodiscard]] virtual std::string toString() const { return getName(); }

		[[nodiscard]] bool isInCategory(EventCategory category) const
		{
			return getCategoryFlags() & category;
		}
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		// F will be deduced by the compiler
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.getEventType() == T::getStaticType())
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.toString();
	}
}

template <>
struct fmt::formatter<Vectrix::Event> : fmt::formatter<std::string> {
	template <typename FormatContext>
	auto format(const Vectrix::Event& e, FormatContext& ctx) const {
		return fmt::formatter<std::string>::format(e.toString(), ctx);
	}
};