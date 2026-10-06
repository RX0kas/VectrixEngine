#pragma once

#include "Event.h"
#include "Vectrix/Input/KeyMods.h"

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
		 * @param mods The modifier keys held
		 * @param scancode The platform-specific code of the physical key
		 */
		KeyPressedEvent(int key, bool repeat, KeyMods mods = {}, int scancode = 0)
			: key(key), repeat(repeat), mods(mods), scancode(scancode) {}

		int key;      ///< The key, matching the `VC_KEY_*` values (see KeyCodes.h), from the US layout
		bool repeat;  ///< True when the key is held and the OS repeats it, false on the first press
		KeyMods mods; ///< The modifier keys held, for shortcuts (Ctrl+S...)
		/// The platform-specific code of the physical key, the same whatever the keyboard layout: to save a
		/// binding to a key's position rather than to what is printed on it
		int scancode;

		[[nodiscard]] std::string toString() const override {
			return fmt::format("KeyPressed: {}{}{}", mods.toString(), key, repeat ? " (repeat)" : "");
		}
	};

	/**
	 * @brief Sent when a key goes back up
	 * @ingroup events
	 */
	class KeyReleasedEvent : public EventBase<KeyReleasedEvent, "KeyReleased", EventCategory::Keyboard | EventCategory::Input> {
	public:
		/**
		 * @param key The key that went up
		 * @param mods The modifier keys held
		 * @param scancode The platform-specific code of the physical key
		 */
		explicit KeyReleasedEvent(int key, KeyMods mods = {}, int scancode = 0) : key(key), mods(mods), scancode(scancode) {}

		int key;      ///< The key, matching the `VC_KEY_*` values (see KeyCodes.h), from the US layout
		KeyMods mods; ///< The modifier keys still held
		int scancode; ///< The platform-specific code of the physical key (see KeyPressedEvent::scancode)

		[[nodiscard]] std::string toString() const override { return fmt::format("KeyReleased: {}{}", mods.toString(), key); }
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
