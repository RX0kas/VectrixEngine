#pragma once

#include "Event.h"

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
		 */
		MouseMovedEvent(float x, float y) : x(x), y(y) {}

		float x; ///< The horizontal position, in screen coordinates from the left of the window
		float y; ///< The vertical position, in screen coordinates from the top of the window

		[[nodiscard]] std::string toString() const override { return fmt::format("MouseMoved: {}, {}", x, y); }
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
		/// @param button The button that went down
		explicit MouseButtonPressedEvent(int button) : button(button) {}

		int button; ///< The button, matching the `VC_MOUSE_BUTTON_*` values (see MouseCodes.h)

		[[nodiscard]] std::string toString() const override { return fmt::format("MouseButtonPressed: {}", button); }
	};

	/**
	 * @brief Sent when a mouse button goes back up
	 * @ingroup events
	 */
	class MouseButtonReleasedEvent : public EventBase<MouseButtonReleasedEvent, "MouseButtonReleased",
			EventCategory::Mouse | EventCategory::MouseButton | EventCategory::Input> {
	public:
		/// @param button The button that went up
		explicit MouseButtonReleasedEvent(int button) : button(button) {}

		int button; ///< The button, matching the `VC_MOUSE_BUTTON_*` values (see MouseCodes.h)

		[[nodiscard]] std::string toString() const override { return fmt::format("MouseButtonReleased: {}", button); }
	};
}
