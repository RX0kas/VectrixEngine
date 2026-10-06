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
	 * @brief Sent when the window's framebuffer changed size, in pixels
	 *
	 * It is the size to give a viewport or an offscreen framebuffer matching the window. It differs from
	 * WindowResizeEvent's on a scaled display (HiDPI, Wayland scale factor).
	 * @note 0x0 while the window is minimized. The renderer recreates its swap chain by itself
	 * @ingroup events
	 */
	class FramebufferResizeEvent : public EventBase<FramebufferResizeEvent, "FramebufferResize", EventCategory::Window> {
	public:
		/**
		 * @param width The new width in pixels
		 * @param height The new height in pixels
		 */
		FramebufferResizeEvent(unsigned int width, unsigned int height) : width(width), height(height) {}

		unsigned int width;  ///< The new width, in pixels
		unsigned int height; ///< The new height, in pixels

		[[nodiscard]] std::string toString() const override {
			return fmt::format("FramebufferResize: {}, {}", width, height);
		}
	};

	/**
	 * @brief Sent when the window's content scale changed, e.g. moved to a screen with another scaling
	 *
	 * The scale is the ratio between the screen's DPI and the platform's default one: 1.0 on a regular
	 * screen, 2.0 on a 200% HiDPI one. Use it to scale the UI and fonts.
	 * @ingroup events
	 */
	class WindowContentScaleEvent : public EventBase<WindowContentScaleEvent, "WindowContentScale", EventCategory::Window> {
	public:
		/**
		 * @param xScale The new horizontal scale
		 * @param yScale The new vertical scale
		 */
		WindowContentScaleEvent(float xScale, float yScale) : xScale(xScale), yScale(yScale) {}

		float xScale; ///< The new horizontal content scale, 1.0 being the platform's default
		float yScale; ///< The new vertical content scale, 1.0 being the platform's default

		[[nodiscard]] std::string toString() const override {
			return fmt::format("WindowContentScale: {}, {}", xScale, yScale);
		}
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
