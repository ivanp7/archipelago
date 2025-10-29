/**
 * @file
 * @brief Application context interface for SDL windows (CPU renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_WINDOW_CPU_VAR_H_
#define _ARCHI_SDL2_CTX_WINDOW_CPU_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief SDL window (+ CPU renderer) context initialization function.
 *
 * Accepts the following parameters:
 * - "params"           : window creation parameters structure
 * - "texture_width"    : texture width in pixels
 * - "texture_height"   : texture height in pixels
 * - "window_width"     : window width in pixels
 * - "window_height"    : window height in pixels
 * - "window_flags"     : window creation flags
 * - "window_title"     : window title
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_cpu_init);

/**
 * @brief SDL window (+ CPU renderer) context finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_cpu_final);

/**
 * @brief SDL window (+ CPU renderer) context getter function.
 *
 * Provides the following slots:
 * - "window"               : SDL window handle
 * - "renderer"             : SDL renderer handle
 * - "texture"              : SDL texture handle
 * - "texture.width"        : texture width in pixels
 * - "texture.height"       : texture height in pixels
 * - "texture.lock"         : pixels of locked texture region
 * - "texture.lock.pitch"   : pitch of locked texture region
 * - "texture.lock.x"       : X coordinate of locked texture region
 * - "texture.lock.y"       : Y coordinate of locked texture region
 * - "texture.lock.width"   : width of locked texture region
 * - "texture.lock.height"  : height of locked texture region
 */
ARCHI_CONTEXT_GET_FUNC(archi_context_sdl2_window_cpu_get);

/**
 * @brief SDL window (+ CPU renderer) context interface.
 */
extern
const archi_context_interface_t archi_context_sdl2_window_cpu_interface;

#endif // _ARCHI_SDL2_CTX_WINDOW_CPU_VAR_H_

