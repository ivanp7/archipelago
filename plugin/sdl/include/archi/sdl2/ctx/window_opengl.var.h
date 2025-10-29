/**
 * @file
 * @brief Application context interface for SDL windows (OpenGL renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_WINDOW_OPENGL_VAR_H_
#define _ARCHI_SDL2_CTX_WINDOW_OPENGL_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief SDL window (+ OpenGL renderer) context initialization function.
 *
 * Accepts the following parameters:
 * - "params"           : window creation parameters structure
 * - "window_width"     : window width in pixels
 * - "window_height"    : window height in pixels
 * - "window_flags"     : window creation flags
 * - "window_title"     : window title
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_opengl_init);

/**
 * @brief SDL window (+ CPU renderer) context finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_opengl_final);

/**
 * @brief SDL window (+ OpenGL renderer) context getter function.
 *
 * Provides the following slots:
 * - "window"   : SDL window handle
 * - "opengl"   : OpenGL context
 */
ARCHI_CONTEXT_GET_FUNC(archi_context_sdl2_window_opengl_get);

/**
 * @brief SDL window (+ CPU renderer) context interface.
 */
extern
const archi_context_interface_t archi_context_sdl2_window_opengl_interface;

#endif // _ARCHI_SDL2_CTX_WINDOW_OPENGL_VAR_H_

