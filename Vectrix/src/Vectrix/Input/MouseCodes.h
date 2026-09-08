#pragma once

/**
 * @file MouseCodes.h
 * @brief The mouse button codes understood by Input::isMouseButtonPressed
 * @ingroup input
 *
 * The values match the ones GLFW uses. The three named buttons below are aliases of the
 * numbered ones, prefer them since they say what they mean.
 * @see Input::isMouseButtonPressed
 * @see MouseButtonEvent::getMouseButton
 */

#define VC_MOUSE_BUTTON_1         0    ///< The first mouse button, the left one on a standard mouse
#define VC_MOUSE_BUTTON_2         1    ///< The second mouse button, the right one on a standard mouse
#define VC_MOUSE_BUTTON_3         2    ///< The third mouse button, the wheel on a standard mouse
#define VC_MOUSE_BUTTON_4         3    ///< The fourth mouse button
#define VC_MOUSE_BUTTON_5         4    ///< The fifth mouse button
#define VC_MOUSE_BUTTON_6         5    ///< The sixth mouse button
#define VC_MOUSE_BUTTON_7         6    ///< The seventh mouse button
#define VC_MOUSE_BUTTON_8         7    ///< The eighth mouse button
/**
 * @brief The left mouse button
 * @ingroup input
 */
#define VC_MOUSE_BUTTON_LEFT      VC_MOUSE_BUTTON_1
/**
 * @brief The right mouse button
 * @ingroup input
 */
#define VC_MOUSE_BUTTON_RIGHT     VC_MOUSE_BUTTON_2
/**
 * @brief The middle mouse button, the wheel pressed down
 * @ingroup input
 */
#define VC_MOUSE_BUTTON_MIDDLE    VC_MOUSE_BUTTON_3