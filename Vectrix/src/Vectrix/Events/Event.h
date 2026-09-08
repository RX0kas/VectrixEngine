#pragma once

#include <string>
#include <ostream>
#include "Vectrix/Core/Core.h"
#include "Vectrix/Core/Log.h"

/**
 * @file Event.h
 * @brief Definition of the Event base class, its categories and the EventDispatcher
 * @ingroup events
 */

namespace Vectrix {
	// TODO : Events should not block the entire app

	/**
	 * @brief Identifies what an Event actually is
	 *
	 * Every concrete event reports its type through Event::getEventType, which lets
	 * EventDispatcher route an event to the right handler.
	 * @ingroup events
	 */
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		SceneSaved, SceneLoading, SceneLoaded
	};

	/**
	 * @brief The families an Event can belong to
	 *
	 * The values are bit flags, so a single event can be part of several categories at
	 * once. A KeyPressedEvent for example is both #EventCategoryKeyboard and
	 * #EventCategoryInput.
	 * @see Event::isInCategory
	 * @ingroup events
	 */
	enum EventCategory
	{
		None = 0,
		EventCategoryWindow = BIT(0),      ///< Sent by the window itself (resize, close, move)
		EventCategoryInput = BIT(1),       ///< Sent by any input device
		EventCategoryKeyboard = BIT(2),    ///< Sent by the keyboard
		EventCategoryMouse = BIT(3),       ///< Sent by the mouse
		EventCategoryMouseButton = BIT(4), ///< Sent by a mouse button specifically
		EventCategoryEditor = BIT(5)       ///< Sent by the editor (scene saved, scene loaded, ...)
	};

	/**
	 * @brief Implements the type related overrides of Event for a concrete event class
	 *
	 * Place it in the public part of an Event subclass, passing the EventType value
	 * without its enum scope. It defines `getStaticType`, `getEventType` and `getName`.
	 * @param type The EventType member matching the class, for example `KeyPressed`
	 */
#define EVENT_CLASS_TYPE(type) \
	static EventType getStaticType() { return EventType::type; } \
	virtual EventType getEventType() const override { return getStaticType(); } \
	virtual const char* getName() const override { return #type; }


	/**
	 * @brief Implements Event::getCategoryFlags for a concrete event class
	 * @param category One or more EventCategory flags combined with `|`
	 */
#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }

	/**
	 * @brief Wraps a member function into an event callback that forwards its return value
	 *
	 * Use it for handlers returning `bool`, so the dispatcher can mark the event handled.
	 * @param fn The member function to bind, for example `MyLayer::onKeyPressed`
	 * @see EventDispatcher::Dispatch
	 */
#define VC_BIND_EVENT_FN_RETURN(fn) [this](auto && PH1) { return fn(std::forward<decltype(PH1)>(PH1)); }

	/**
	 * @brief Wraps a member function into an event callback that discards its return value
	 * @param fn The member function to bind
	 */
#define VC_BIND_EVENT_FN(fn) [this](auto && PH1) { fn(std::forward<decltype(PH1)>(PH1)); } // TODO: Fix event system

	/**
	 * @brief Base class of everything the engine reports to the layers
	 *
	 * Events are created by the window or the editor and are passed down the layer stack
	 * through Layer::OnEvent. A layer that consumes an event sets #Handled so the layers
	 * below it stop receiving it.
	 * @see EventDispatcher
	 * @see Layer::OnEvent
	 * @ingroup events
	 */
	class Event {
	public:
		virtual ~Event() = default;

		/**
		 * @brief Whether a layer already took care of this event
		 *
		 * Once it is true the event should not be processed again by the layers below.
		 */
		bool Handled = false;

		/**
		 * @brief Return what kind of event this is
		 * @return The EventType of the concrete event
		 */
		[[nodiscard]] virtual EventType getEventType() const = 0;

		/**
		 * @brief Return the name of the event, mostly useful for logging
		 * @return The event name as a null terminated string
		 */
		[[nodiscard]] virtual const char* getName() const = 0;

		/**
		 * @brief Return every EventCategory this event belongs to
		 * @return The categories combined into a bit field
		 * @see isInCategory
		 */
		[[nodiscard]] virtual int getCategoryFlags() const = 0;

		/**
		 * @brief Describe the event and its payload in a human readable way
		 * @return A string suited for logging
		 */
		[[nodiscard]] virtual std::string toString() const = 0;

		/**
		 * @brief Tell if the event belongs to a given category
		 * @param category The category to test
		 * @return true when the event carries that category flag
		 */
		[[nodiscard]] bool isInCategory(EventCategory category) const {
			return getCategoryFlags() & category;
		}
	};

	/**
	 * @brief Calls a handler only when the event matches the expected type
	 *
	 * Wrap the received event into a dispatcher, then call Dispatch once per event type
	 * the layer cares about.
	 * @code
	 * void MyLayer::OnEvent(Event& event) {
	 *     EventDispatcher dispatcher(event);
	 *     dispatcher.Dispatch<KeyPressedEvent>(VC_BIND_EVENT_FN_RETURN(MyLayer::onKeyPressed));
	 * }
	 * @endcode
	 * @ingroup events
	 */
	class EventDispatcher
	{
	public:
		/**
		 * @brief Build a dispatcher around the event being processed
		 * @param event The event to dispatch, it has to outlive the dispatcher
		 */
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		/**
		 * @brief Run the handler when the event is of type T
		 *
		 * The value returned by the handler is merged into Event::Handled, so returning
		 * true stops the event from reaching the layers below.
		 * @tparam T The event class the handler expects
		 * @tparam F The callable type, deduced by the compiler
		 * @param func The handler, taking a `T&` and returning a bool
		 * @return true when the event matched T and the handler ran
		 */
		// F will be deduced by the compiler
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.getEventType() == T::getStaticType())
			{
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	/**
	 * @brief Write the description of an event into a stream
	 * @param os The destination stream
	 * @param e The event to describe
	 * @return The stream, so the calls can be chained
	 * @see Event::toString
	 */
	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		os << e.toString();
		os.flush();
		return os;
	}
}

/// @cond INTERNAL
template <>
struct fmt::formatter<Vectrix::Event> : fmt::formatter<std::string> {
	template <typename FormatContext>
	auto format(const Vectrix::Event& e, FormatContext& ctx) const {
		return fmt::formatter<std::string>::format(e.toString(), ctx);
	}
};
/// @endcond
