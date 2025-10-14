/**
 * @file
 * @brief Types for operations with SDL windows (OpenGL renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_API_WINDOW_OPENGL_TYP_H_
#define _ARCHI_SDL2_API_WINDOW_OPENGL_TYP_H_

#include "archi/sdl2/api/window.typ.h"

/**
 * @brief Parameters for archi_sdl2_window_opengl_create().
 *
 * If window width is 0, it is substituted with texture width.
 * If window height is 0, it is substituted with texture height.
 */
typedef struct archi_sdl2_window_opengl_params {
    archi_sdl2_window_params_t window; ///< Window parameters.
} archi_sdl2_window_opengl_params_t;

#endif // _ARCHI_SDL2_API_WINDOW_OPENGL_TYP_H_

