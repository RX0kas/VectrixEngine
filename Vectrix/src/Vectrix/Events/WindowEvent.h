#pragma once

#include <vector>

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
	 * The Application stops once every listener got it without consuming it: a layer whose handler returns
	 * true keeps the application running (e.g. to ask first about unsaved work).
	 * @see Application::close
	 * @ingroup events
	 */
	class WindowCloseEvent : public EventBase<WindowCloseEvent, "WindowClose", EventCategory::Window> {
	public:
		WindowCloseEvent() = default;
	};

	/**
	 * @brief Sent when the window changed size
	 * @note The size is in screen coordinates, which differ from the framebuffer's pixels on a scaled
	 *       display; the renderer follows the framebuffer by itself
	 * @ingroup events
	 */
	class WindowResizeEvent : public EventBase<WindowResizeEvent, "WindowResize", EventCategory::Window> {
	public:
		/**
		 * @param width The new width
		 * @param height The new height
		 */
		WindowResizeEvent(unsigned int width, unsigned int height) : width(width), height(height) {}

		unsigned int width;  ///< The new width, in screen coordinates
		unsigned int height; ///< The new height, in screen coordinates

		[[nodiscard]] std::string toString() const override { return fmt::format("WindowResize: {}, {}", width, height); }
	};

	/**
	 * @brief Sent when the window gains or loses the keyboard focus
	 * @ingroup events
	 */
	class WindowFocusEvent : public EventBase<WindowFocusEvent, "WindowFocus", EventCategory::Window> {
	public:
		/// @param focused Whether the window now has the focus
		explicit WindowFocusEvent(bool focused) : focused(focused) {}

		bool focused; ///< True when the window gained the focus, false when it lost it

		[[nodiscard]] std::string toString() const override { return fmt::format("WindowFocus: {}", focused); }
	};

	/**
	 * @brief Sent when the window moved on the screen
	 * @note Never sent on Wayland, where a window can't know its position
	 * @ingroup events
	 */
	class WindowMovedEvent : public EventBase<WindowMovedEvent, "WindowMoved", EventCategory::Window> {
	public:
		/**
		 * @param x The new position of the window's left edge
		 * @param y The new position of the window's top edge
		 */
		WindowMovedEvent(int x, int y) : x(x), y(y) {}

		int x; ///< The position of the window's left edge, in screen coordinates
		int y; ///< The position of the window's top edge, in screen coordinates

		[[nodiscard]] std::string toString() const override { return fmt::format("WindowMoved: {}, {}", x, y); }
	};

	/**
	 * @brief Sent when the window is minimized, or restored from it
	 * @note The application doesn't render while minimized
	 * @ingroup events
	 */
	class WindowMinimizeEvent : public EventBase<WindowMinimizeEvent, "WindowMinimize", EventCategory::Window> {
	public:
		/// @param minimized Whether the window is now minimized
		explicit WindowMinimizeEvent(bool minimized) : minimized(minimized) {}

		bool minimized; ///< True when the window was minimized, false when it was restored

		[[nodiscard]] std::string toString() const override { return fmt::format("WindowMinimize: {}", minimized); }
	};

	/**
	 * @brief Sent when files or folders are dropped onto the window
	 * @ingroup events
	 */
	class FilesDroppedEvent : public EventBase<FilesDroppedEvent, "FilesDropped", EventCategory::Window> {
	public:
		/// @param paths The dropped paths, in UTF-8
		explicit FilesDroppedEvent(std::vector<std::string> paths) : paths(std::move(paths)) {}

		std::vector<std::string> paths; ///< The absolute paths dropped, in UTF-8 (turn them into paths with fromUtf8)

		[[nodiscard]] std::string toString() const override {
			return fmt::format("FilesDropped: {} path(s)", paths.size());
		}
	};
}
