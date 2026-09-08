#pragma once
#include "Vectrix/Events/Event.h"

/**
 * @file WindowEvent.h
 * @brief Definition of the events sent by the window itself
 * @ingroup events
 */

namespace Vectrix {
	/**
	 * @brief Sent when the user asks to close the window
	 *
	 * The Application stops its main loop once this event is handled.
	 * @ingroup events
	 */
	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryWindow)

		/// @copydoc Event::toString
		[[nodiscard]] std::string toString() const override	{
			return getName();
		}
	};

	/**
	 * @brief Sent when the drawable area of the window changed size
	 *
	 * The renderer uses it to recreate the swapchain, so a layer reacting to it should
	 * not consume the event unless it really means to.
	 * @ingroup events
	 */
	class WindowResizeEvent : public Event {
	public:
		/**
		 * @brief Build the event with the new window size
		 * @param width The new width in pixel
		 * @param height The new height in pixel
		 */
		WindowResizeEvent(unsigned int width, unsigned int height) : m_Width(width), m_Height(height) {}

		/**
		 * @brief Return the new width of the window
		 * @return The width in pixel
		 */
		[[nodiscard]] unsigned int getWidth() const { return m_Width; }

		/**
		 * @brief Return the new height of the window
		 * @return The height in pixel
		 */
		[[nodiscard]] unsigned int getHeight() const { return m_Height; }

		/// @copydoc Event::toString
		[[nodiscard]] std::string toString() const override	{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryWindow)
	private:
		unsigned int m_Width, m_Height;
	};
}
