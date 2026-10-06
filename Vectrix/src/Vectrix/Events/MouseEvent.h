#pragma once

#include "Event.h"
#include "Vectrix/Input/KeyMods.h"

/**
 * @file MouseEvent.h
 * @brief Definition of the mouse events
 * @ingroup events
 */

namespace Vectrix {
	/**
	 * @brief Sent when the cursor moved over the window
	 * @ingroup events
	 */
	class MouseMovedEvent : public EventBase<MouseMovedEvent, "MouseMoved", EventCategory::Mouse | EventCategory::Input> {
	public:
		/**
		 * @param x The horizontal position, from the left of the window
		 * @param y The vertical position, from the top of the window
		 * @param dx How far the cursor moved horizontally since the previous MouseMovedEvent
		 * @param dy How far the cursor moved vertically since the previous MouseMovedEvent
		 */
		MouseMovedEvent(float x, float y, float dx = 0.0f, float dy = 0.0f) : x(x), y(y), dx(dx), dy(dy) {}

		float x;  ///< The horizontal position, in screen coordinates from the left of the window
		float y;  ///< The vertical position, in screen coordinates from the top of the window
		/// How far the cursor moved since the previous MouseMovedEvent, positive to the right. 0 for the first
		/// move after the cursor entered the window, so coming back in at another place isn't a jump
		float dx;
		float dy; ///< How far the cursor moved since the previous MouseMovedEvent, positive downwards (see dx)

		[[nodiscard]] std::string toString() const override {
			return fmt::format("MouseMoved: {}, {} ({:+}, {:+})", x, y, dx, dy);
		}
	};

	/**
	 * @brief Sent when the wheel is scrolled, or when a touchpad reports a scroll
	 * @ingroup events
	 */
	class MouseScrolledEvent : public EventBase<MouseScrolledEvent, "MouseScrolled", EventCategory::Mouse | EventCategory::Input> {
	public:
		/**
		 * @param xOffset The horizontal scroll, negative to the left
		 * @param yOffset The vertical scroll, negative when scrolling down
		 */
		MouseScrolledEvent(float xOffset, float yOffset) : xOffset(xOffset), yOffset(yOffset) {}

		float xOffset; ///< The horizontal scroll, negative to the left
		float yOffset; ///< The vertical scroll, negative when scrolling down

		[[nodiscard]] std::string toString() const override {
			return fmt::format("MouseScrolled: {}, {}", xOffset, yOffset);
		}
	};

	/**
	 * @brief Sent when a mouse button goes down
	 * @ingroup events
	 */
	class MouseButtonPressedEvent : public EventBase<MouseButtonPressedEvent, "MouseButtonPressed",
			EventCategory::Mouse | EventCategory::MouseButton | EventCategory::Input> {
	public:
		/**
		 * @param button The button that went down
		 * @param mods The modifier keys held
		 */
		explicit MouseButtonPressedEvent(int button, KeyMods mods = {}) : button(button), mods(mods) {}

		int button;   ///< The button, matching the `VC_MOUSE_BUTTON_*` values (see MouseCodes.h)
		KeyMods mods; ///< The modifier keys held (Ctrl+click...)

		[[nodiscard]] std::string toString() const override {
			return fmt::format("MouseButtonPressed: {}{}", mods.toString(), button);
		}
	};

	/**
	 * @brief Sent when a mouse button goes back up
	 * @ingroup events
	 */
	class MouseButtonReleasedEvent : public EventBase<MouseButtonReleasedEvent, "MouseButtonReleased",
			EventCategory::Mouse | EventCategory::MouseButton | EventCategory::Input> {
	public:
		/**
		 * @param button The button that went up
		 * @param mods The modifier keys held
		 */
		explicit MouseButtonReleasedEvent(int button, KeyMods mods = {}) : button(button), mods(mods) {}

		int button;   ///< The button, matching the `VC_MOUSE_BUTTON_*` values (see MouseCodes.h)
		KeyMods mods; ///< The modifier keys still held

		[[nodiscard]] std::string toString() const override {
			return fmt::format("MouseButtonReleased: {}{}", mods.toString(), button);
		}
	};

	/**
	 * @brief Sent when the cursor enters the window, or leaves it
	 * @note Never swallowed by ImGui (it isn't in the Mouse category), so every enter has its leave
	 * @ingroup events
	 */
	class MouseEnterEvent : public EventBase<MouseEnterEvent, "MouseEnter", EventCategory::Window | EventCategory::Input> {
	public:
		/// @param entered Whether the cursor is now over the window
		explicit MouseEnterEvent(bool entered) : entered(entered) {}

		bool entered; ///< True when the cursor entered the window, false when it left it

		[[nodiscard]] std::string toString() const override { return fmt::format("MouseEnter: {}", entered); }
	};
}
