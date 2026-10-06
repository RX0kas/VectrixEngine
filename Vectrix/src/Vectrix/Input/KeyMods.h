#pragma once

#include <string>

/**
 * @file KeyMods.h
 * @brief Definition of KeyMods, the modifier keys held during a key or mouse button event
 * @ingroup input
 */

namespace Vectrix {
	/**
	 * @brief The modifier keys held when a key or a mouse button event happened
	 *
	 * Either side counts: ctrl is true for the left and the right Control key alike.
	 * @code
	 * subscribe<KeyPressedEvent>([this](const KeyPressedEvent& e) {
	 *     if (e.key != VC_KEY_S || e.mods != KeyMods{.ctrl = true}) return false;
	 *     save();
	 *     return true;
	 * });
	 * @endcode
	 * @note capsLock and numLock tell whether those locks are on, and are left out of the comparisons
	 * @see KeyPressedEvent::mods
	 * @see MouseButtonPressedEvent::mods
	 * @ingroup input
	 */
	struct KeyMods {
		bool shift = false;    ///< A Shift key is held
		bool ctrl = false;     ///< A Control key is held
		bool alt = false;      ///< An Alt key is held
		bool super = false;    ///< A Super key is held (Windows key, Command on macOS)
		bool capsLock = false; ///< Caps Lock is on
		bool numLock = false;  ///< Num Lock is on

		/// @brief Tell if no modifier key is held (the locks don't count)
		[[nodiscard]] constexpr bool none() const { return !shift && !ctrl && !alt && !super; }

		/// @brief Compare the held modifier keys, ignoring the locks, so `KeyMods{.ctrl = true}` matches Ctrl alone
		[[nodiscard]] constexpr bool operator==(const KeyMods& other) const {
			return shift == other.shift && ctrl == other.ctrl && alt == other.alt && super == other.super;
		}

		/// @brief Describe the held modifiers, e.g. "Ctrl+Shift+", empty when there are none
		[[nodiscard]] std::string toString() const {
			std::string text;
			if (ctrl) text += "Ctrl+";
			if (shift) text += "Shift+";
			if (alt) text += "Alt+";
			if (super) text += "Super+";
			return text;
		}
	};
}
