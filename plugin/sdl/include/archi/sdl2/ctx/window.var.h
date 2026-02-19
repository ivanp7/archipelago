/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Context interface for SDL windows.
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_WINDOW_VAR_H_
#define _ARCHI_SDL2_CTX_WINDOW_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: SDL window.
 *
 * Initialization parameters:
 * - "title"            : (char[]) window title
 * - "width"            : (int) window width
 * - "height"           : (int) window height
 * - "coord_x"          : (int) horizontal window coordinate
 * - "coord_y"          : (int) vertical window coordinate
 * - "center_x"         : (char) whether window is centered horizontally
 * - "center_y"         : (char) whether window is centered vertically
 * - "fullscreen"       : (char) window is fullscreen
 * - "hidden"           : (char) window is hidden
 * - "borderless"       : (char) no window decoration
 * - "resizable"        : (char) window can be resized
 * - "minimized"        : (char) window is minimized
 * - "maximized"        : (char) window is maximized
 * - "input_focus"      : (char) window has input focus
 * - "mouse_focus"      : (char) window has mouse focus
 * - "keyboard_grabbed" : (char) window has grabbed keyboard input
 * - "mouse_grabbed"    : (char) window has grabbed mouse input
 * - "mouse_capture"    : (char) window has captured mouse
 * - "always_on_top"    : (char) window should always be above others
 * - "skip_taskbar"     : (char) window should not be added to the taskbar
 * - "utility"          : (char) window should be treated as utility window
 * - "tooltip"          : (char) window should be treated as tooltip
 * - "popup_menu"       : (char) window should be treated as popup menu
 * - "opengl"           : (char) window is usable with OpenGL context
 * - "flags"            : (uint32_t) window creation flags
 */
extern
const archi_context_interface_t
archi_context_interface__sdl2_window;

#endif // _ARCHI_SDL2_CTX_WINDOW_VAR_H_

