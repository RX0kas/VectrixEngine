#pragma once

#include "Event.h"

/**
 * @file KeyEvent.h
 * @brief Definition of the keyboard events
 * @ingroup events
 */

namespace Vectrix {
	/**
	 * @brief Base class of every event coming from the keyboard
	 *
	 * It carries the key involved, the concrete subclasses tell what happened to it.
	 * @see KeyPressedEvent
	 * @see KeyReleasedEvent
	 * @ingroup events
	 */
	class KeyEvent : public Event {
	public:
		/**
		 * @brief Return the key the event is about
		 * @return The key code, matching the `VC_KEY_*` values
		 * @see KeyCodes.h
		 */
		[[nodiscard]] int getKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
		/// @copydoc Event::toString
		[[nodiscard]] std::string toString() const override { return getName(); }
	protected:
		/**
		 * @brief Build a keyboard event for a given key
		 * @param keycode The key the event is about
		 */
		KeyEvent(int keycode) : m_KeyCode(keycode) {}

		/// The key the event is about
		int m_KeyCode;
	};

	/**
	 * @brief Sent when a key goes down, and again while it repeats
	 * @ingroup events
	 */
	class KeyPressedEvent : public KeyEvent {
	public:
		/**
		 * @brief Build the event for a key that has just been pressed
		 * @param keycode The key that went down
		 * @param repeatCount How many times the key repeated, 0 on the first press
		 */
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

		/**
		 * @brief Return how many times the key repeated
		 * @return 0 for the initial press, then the number of repeats sent by the OS
		 */
		[[nodiscard]] int getRepeatCount() const { return m_RepeatCount; }

		/// @copydoc Event::toString
		[[nodiscard]] std::string toString() const override	{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	/**
	 * @brief Sent when a key goes back up
	 * @ingroup events
	 */
	class KeyReleasedEvent : public KeyEvent {
	public:
		/**
		 * @brief Build the event for a key that has just been released
		 * @param keycode The key that went up
		 */
		KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

		/// @copydoc Event::toString
		[[nodiscard]] std::string toString() const override	{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};
}
