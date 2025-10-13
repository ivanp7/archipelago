/**
 * @file
 * @brief Common types for operations with SDL windows.
 */

#pragma once
#ifndef _ARCHI_SDL2_API_WINDOW_TYP_H_
#define _ARCHI_SDL2_API_WINDOW_TYP_H_

#include <stdint.h> // for uint32_t

/**
 * @brief Parameters of window creation.
 */
typedef struct archi_sdl2_window_params {
    int width;  ///< Window width in pixels.
    int height; ///< Window height in pixels.

    uint32_t flags; ///< Window flags.

    const char *title; ///< Window title.
} archi_sdl2_window_params_t;

#endif // _ARCHI_SDL2_API_WINDOW_TYP_H_

