/**
 * @file
 * @brief Application context interface for SDL windows.
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_WINDOW_VAR_H_
#define _ARCHI_SDL2_CTX_WINDOW_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief SDL window context initialization function.
 *
 * Accepts the following parameters:
 * - "fullscreen"       : window is fullscreen
 * - "hidden"           : window is hidden
 * - "borderless"       : no window decoration
 * - "resizable"        : window can be resized
 * - "minimized"        : window is minimized
 * - "maximized"        : window is maximized
 * - "input_focus"      : window has input focus
 * - "mouse_focus"      : window has mouse focus
 * - "keyboard_grabbed" : window has grabbed keyboard input
 * - "mouse_grabbed"    : window has grabbed mouse input
 * - "mouse_capture"    : window has captured mouse
 * - "always_on_top"    : window should always be above others
 * - "skip_taskbar"     : window should not be added to the taskbar
 * - "utility"          : window should be treated as utility window
 * - "tooltip"          : window should be treated as tooltip
 * - "popup_menu"       : window should be treated as popup menu
 * - "opengl"           : window is usable with OpenGL context
 * - "flags"            : window creation flags
 * - "width"            : window width
 * - "height"           : window height
 * - "coord_x"          : horizontal window coordinate
 * - "coord_y"          : vertical window coordinate
 * - "center_x"         : whether window is centered horizontally
 * - "center_y"         : whether window is centered vertically
 * - "title"            : window title
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_init);

/**
 * @brief SDL window context finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_final);

/**
 * @brief SDL window context interface.
 */
extern
const archi_context_interface_t archi_context_sdl2_window_interface;

#endif // _ARCHI_SDL2_CTX_WINDOW_VAR_H_

