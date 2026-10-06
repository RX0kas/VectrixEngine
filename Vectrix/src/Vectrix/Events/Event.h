#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include "Vectrix/Core/Core.h"
#include "Vectrix/Core/Log.h"

/**
 * @file Event.h
 * @brief Definition of the Event base class, its categories and EventBase, the base of every concrete event
 * @ingroup events
 */

namespace Vectrix {
	/**
	 * @brief The families an Event can belong to
	 *
	 * The values are bit flags, combined with `|`: a KeyPressedEvent is both #Keyboard and #Input.
	 * @see Event::isInCategory
	 * @ingroup events
	 */
	enum class EventCategory : uint8_t {
		None = 0,
		Window = BIT(0), ///< Sent by the window itself (resize, close, move, focus, files dropped)
		Input = BIT(1), ///< Sent by any input device
		Keyboard = BIT(2), ///< Sent by the keyboard (keys and typed text)
		Mouse = BIT(3), ///< Sent by the mouse
		MouseButton = BIT(4), ///< Sent by a mouse button specifically
	};

	/// @brief Combine two sets of categories
	constexpr EventCategory operator|(EventCategory a, EventCategory b) {
		return static_cast<EventCategory>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
	}
	/// @brief Keep the categories two sets have in common
	constexpr EventCategory operator&(EventCategory a, EventCategory b) {
		return static_cast<EventCategory>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
	}

	/**
	 * @brief Identifies the concrete class of an Event
	 *
	 * Generated per class by eventTypeId, so an application can define its own events without touching an
	 * engine enum. Only meaningful for comparisons.
	 * @ingroup events
	 */
	using EventTypeId = const void*;

	/// @cond INTERNAL
	namespace detail {
		// One variable per event class, its address is the class id (unique across the whole program)
		template<typename T>
		inline constexpr char eventTypeTag = 0;

		// A string literal usable as a template argument, for EventBase's name
		template<size_t N>
		struct EventName {
			char value[N]{};
			constexpr EventName(const char (&str)[N]) { std::copy_n(str, N, value); }
			[[nodiscard]] constexpr std::string_view view() const { return {value, N - 1}; }
		};
	}
	/// @endcond

	/**
	 * @brief Return the id of an event class
	 * @tparam T The event class
	 * @return The same value for every T event, different from every other class
	 * @ingroup events
	 */
	template<typename T>
	constexpr EventTypeId eventTypeId() { return &detail::eventTypeTag<std::remove_cvref_t<T>>; }

	/**
	 * @brief Base class of everything the engine, or the application, reports to the event listeners
	 *
	 * An event is not derived from directly: use EventBase, which fills in its type, name and categories.
	 * Listeners receive the concrete class (`const KeyPressedEvent&`), so is/as are mostly useful in a
	 * handler subscribed to every event (`subscribe<Event>`).
	 * @see EventBase
	 * @see EventListener
	 * @ingroup events
	 */
	class Event {
	public:
		virtual ~Event() = default;

		/// @brief Return the id of the event's class, the value eventTypeId gives for it
		[[nodiscard]] EventTypeId getType() const { return m_type; }

		/// @brief Return the name of the event's class, for logging (e.g. "KeyPressed")
		[[nodiscard]] virtual std::string_view getName() const = 0;

		/// @brief Return every category the event belongs to
		[[nodiscard]] EventCategory getCategories() const { return m_categories; }

		/**
		 * @brief Tell if the event belongs to one of the given categories
		 * @param category One or more categories, combined with `|`
		 */
		[[nodiscard]] bool isInCategory(EventCategory category) const {
			return (m_categories & category) != EventCategory::None;
		}

		/**
		 * @brief Tell if a listener consumed the event
		 *
		 * A handler consumes an event by returning true; the listeners after it don't receive it.
		 */
		[[nodiscard]] bool isHandled() const { return m_handled; }

		/// @brief Tell if the event is a T
		template<typename T>
		[[nodiscard]] bool is() const { return m_type == eventTypeId<T>(); }

		/// @brief Return the event as a T, or nullptr when it is another event
		template<typename T>
		[[nodiscard]] const T* as() const { return is<T>() ? static_cast<const T*>(this) : nullptr; }

		/**
		 * @brief Describe the event and its payload, for logging
		 * @note Defaults to the name; events carrying data override it
		 */
		[[nodiscard]] virtual std::string toString() const { return std::string(getName()); }
	protected:
		/// @cond INTERNAL
		Event(EventTypeId type, EventCategory categories) : m_type(type), m_categories(categories) {}
		Event(const Event&) = default;
		Event& operator=(const Event&) = default;
		/// @endcond
	private:
		friend class EventListener;
		friend class Application;

		EventTypeId m_type;
		EventCategory m_categories;
		bool m_handled = false;
	};

	/**
	 * @brief The base of every concrete event: gives it its type id, name and categories
	 *
	 * Derive the event from it, naming the class itself, then add the event's data as public members:
	 * @code
	 * struct SceneOpenedEvent : Vectrix::EventBase<SceneOpenedEvent, "SceneOpened"> {
	 *     explicit SceneOpenedEvent(std::string path) : path(std::move(path)) {}
	 *     std::string path;
	 * };
	 *
	 * Application::instance().postEvent<SceneOpenedEvent>(scenePath);
	 * @endcode
	 * @tparam Derived The event class being defined
	 * @tparam Name The name getName returns, for logging
	 * @tparam Categories The categories of the event, combined with `|`
	 * @ingroup events
	 */
	template<typename Derived, detail::EventName Name, EventCategory Categories = EventCategory::None>
	class EventBase : public Event {
	public:
		/// @brief The categories every event of this class belongs to
		static constexpr EventCategory staticCategories = Categories;

		[[nodiscard]] std::string_view getName() const override { return Name.view(); }
	protected:
		/// @cond INTERNAL
		EventBase() : Event(eventTypeId<Derived>(), Categories) {}
		/// @endcond
	};
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
