#pragma once

#include "vcpch.h"
#include "Vectrix/Core.h"
#include "vulkan/generated/vk_enum_string_helper.h"

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
	static EventType GetStaticType() { return EventType::type; } \
	virtual EventType GetEventType() const override { return GetStaticType(); } \
	virtual const char* GetName() const override { return #type; }


//  A macro to not have to rewrite the getCategoryFlags function every time we create a new event
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }


	class Vectrix_API Event
	{
	public:
		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}

template <>
struct fmt::formatter<Vectrix::Event> : fmt::formatter<std::string> {
	template <typename FormatContext>
	auto format(const Vectrix::Event& e, FormatContext& ctx) const {
		return fmt::formatter<std::string>::format(e.ToString(), ctx);
	}
};

template <>
struct fmt::formatter<VkResult> : fmt::formatter<std::string> {
	template <typename FormatContext>
	auto format(const VkResult& e, FormatContext& ctx) const {
		return fmt::formatter<std::string>::format(string_VkResult(e), ctx);
	}
};