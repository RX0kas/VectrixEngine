#pragma once

#include "vcpch.h"
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
	class MouseMovedEvent : public Event {
	public:
		/**
		 * @brief Build the event with the new cursor position
		 * @param x The horizontal position, in pixel from the left of the window
		 * @param y The vertical position, in pixel from the top of the window
		 */
		MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

		/**
		 * @brief Return the horizontal position of the cursor
		 * @return The position in pixel from the left of the window
		 */
		float getX() const { return m_MouseX; }

		/**
		 * @brief Return the vertical position of the cursor
		 * @return The position in pixel from the top of the window
		 */
		float getY() const { return m_MouseY; }

		/// @copydoc Event::toString
		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_MouseX, m_MouseY;
	};

	/**
	 * @brief Sent when the wheel is scrolled, or when a touchpad reports a scroll
	 * @ingroup events
	 */
	class MouseScrolledEvent : public Event {
	public:
		/**
		 * @brief Build the event with the scroll amount
		 * @param xOffset The horizontal scroll, negative to the left
		 * @param yOffset The vertical scroll, negative when scrolling down
		 */
		MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

		/**
		 * @brief Return how much was scrolled horizontally
		 * @return The offset, negative to the left
		 */
		float getXOffset() const { return m_XOffset; }

		/**
		 * @brief Return how much was scrolled vertically
		 * @return The offset, negative when scrolling down
		 */
		float getYOffset() const { return m_YOffset; }

		/// @copydoc Event::toString
		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << getXOffset() << ", " << getYOffset();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_XOffset, m_YOffset;
	};

	/**
	 * @brief Base class of the events about a mouse button
	 *
	 * It carries the button involved, the concrete subclasses tell what happened to it.
	 * @see MouseButtonPressedEvent
	 * @see MouseButtonReleasedEvent
	 * @ingroup events
	 */
	class MouseButtonEvent : public Event {
	public:
		/**
		 * @brief Return the button the event is about
		 * @return The button code, matching the `VC_MOUSE_BUTTON_*` values
		 * @see MouseCodes.h
		 */
		int getMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	protected:
		/**
		 * @brief Build a mouse button event for a given button
		 * @param button The button the event is about
		 */
		explicit MouseButtonEvent(int button) : m_Button(button) {}

		/// The button the event is about
		int m_Button;
	};

	/**
	 * @brief Sent when a mouse button goes down
	 * @ingroup events
	 */
	class MouseButtonPressedEvent : public MouseButtonEvent	{
	public:
		/**
		 * @brief Build the event for a button that has just been pressed
		 * @param button The button that went down
		 */
		explicit MouseButtonPressedEvent(int button)	: MouseButtonEvent(button) {}

		/// @copydoc Event::toString
		[[nodiscard]] std::string toString() const override {
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	/**
	 * @brief Sent when a mouse button goes back up
	 * @ingroup events
	 */
	class MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
		/**
		 * @brief Build the event for a button that has just been released
		 * @param button The button that went up
		 */
		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

		/// @copydoc Event::toString
		[[nodiscard]] std::string toString() const override {
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}
