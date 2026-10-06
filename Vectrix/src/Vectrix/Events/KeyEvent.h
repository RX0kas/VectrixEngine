#pragma once

#include "Event.h"

/**
 * @file KeyEvent.h
 * @brief Definition of the keyboard events
 * @ingroup events
 */

namespace Vectrix {
	/**
	 * @brief Sent when a key goes down, and again while it is held (repeat)
	 * @ingroup events
	 */
	class KeyPressedEvent : public EventBase<KeyPressedEvent, "KeyPressed", EventCategory::Keyboard | EventCategory::Input> {
	public:
		/**
		 * @param key The key that went down
		 * @param repeat Whether it is the OS repeating a held key rather than the first press
		 */
		KeyPressedEvent(int key, bool repeat) : key(key), repeat(repeat) {}

		int key;     ///< The key, matching the `VC_KEY_*` values (see KeyCodes.h)
		bool repeat; ///< True when the key is held and the OS repeats it, false on the first press

		[[nodiscard]] std::string toString() const override {
			return fmt::format("KeyPressed: {}{}", key, repeat ? " (repeat)" : "");
		}
	};

	/**
	 * @brief Sent when a key goes back up
	 * @ingroup events
	 */
	class KeyReleasedEvent : public EventBase<KeyReleasedEvent, "KeyReleased", EventCategory::Keyboard | EventCategory::Input> {
	public:
		/// @param key The key that went up
		explicit KeyReleasedEvent(int key) : key(key) {}

		int key; ///< The key, matching the `VC_KEY_*` values (see KeyCodes.h)

		[[nodiscard]] std::string toString() const override { return fmt::format("KeyReleased: {}", key); }
	};

	/**
	 * @brief Sent for every character typed, for text input
	 *
	 * Unlike KeyPressedEvent it follows the keyboard layout, the modifiers and dead keys: Shift+A gives 'A',
	 * and keys typing nothing (arrows, Ctrl...) send none.
	 * @ingroup events
	 */
	class KeyTypedEvent : public EventBase<KeyTypedEvent, "KeyTyped", EventCategory::Keyboard | EventCategory::Input> {
	public:
		/// @param codepoint The Unicode code point of the typed character
		explicit KeyTypedEvent(char32_t codepoint) : codepoint(codepoint) {}

		char32_t codepoint; ///< The Unicode code point of the typed character

		[[nodiscard]] std::string toString() const override {
			return fmt::format("KeyTyped: U+{:04X}", static_cast<uint32_t>(codepoint));
		}
	};
}
