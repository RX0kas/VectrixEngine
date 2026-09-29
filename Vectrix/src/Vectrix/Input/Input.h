#pragma once

/**
 * @file Input.h
 * @brief Definition of the Input class, used to poll the keyboard and the mouse
 * @ingroup input
 */

namespace Vectrix {
	/**
	 * @brief Reads the state of the keyboard and the mouse, whenever it is needed
	 *
	 * Every function is static and answers about the current state, which makes it the
	 * counterpart of the event system: use an Event to react to the moment something
	 * changed, and Input to ask whether a key is held right now. The platform provides
	 * the implementation, so nothing has to be created by hand.
	 * @see KeyEvent
	 * @see MouseEvent
	 * @ingroup input
	 */
	class Input	{
	public:
		virtual ~Input() = default;

		/**
		 * @brief Tell if a key is currently held down
		 * @param keycode The key to test, one of the `VC_KEY_*` values
		 * @return true while the key is down
		 * @see KeyCodes.h
		 */
		static bool isKeyPressed(int keycode) { return s_instance->isKeyPressedImpl(keycode); }

		/**
		 * @brief Tell if a mouse button is currently held down
		 * @param button The button to test, one of the `VC_MOUSE_BUTTON_*` values
		 * @return true while the button is down
		 * @see MouseCodes.h
		 */
		static bool isMouseButtonPressed(int button) { return s_instance->isMouseButtonPressedImpl(button); }

		/**
		 * @brief Return where the cursor is inside the window
		 * @return The horizontal and vertical position, in pixel from the top left corner
		 */
		static std::pair<float, float> getMousePosition() { return s_instance->getMousePositionImpl(); }

		/**
		 * @brief Return the horizontal position of the cursor
		 * @return The position in pixel from the left of the window
		 */
		static float getMouseX() { return s_instance->getMouseXImpl(); }

		/**
		 * @brief Return the vertical position of the cursor
		 * @return The position in pixel from the top of the window
		 */
		static float getMouseY() { return s_instance->getMouseYImpl(); }
	protected:
		/// @cond INTERNAL
		virtual bool isKeyPressedImpl(int keycode) = 0;

		virtual bool isMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> getMousePositionImpl() = 0;
		virtual float getMouseXImpl() = 0;
		virtual float getMouseYImpl() = 0;
		/// @endcond
	private:
		static Input* s_instance;
	};
}
