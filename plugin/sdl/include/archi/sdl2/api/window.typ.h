/**
 * @file
 * @brief Types for operations with SDL windows.
 */

#pragma once
#ifndef _ARCHI_SDL2_API_WINDOW_TYP_H_
#define _ARCHI_SDL2_API_WINDOW_TYP_H_

#include <stdint.h> // for uint32_t

/**
 * @brief Parameters for archi_sdl2_window_create().
 *
 * If window width is 0, it is substituted with texture width.
 * If window height is 0, it is substituted with texture height.
 */
typedef struct archi_sdl2_window_params {
    struct {
        int width;  ///< Texture width in pixels.
        int height; ///< Texture height in pixels.
    } texture;

    struct {
        int width;  ///< Window width in pixels.
        int height; ///< Window height in pixels.

        uint32_t flags; ///< Window flags.

        const char *title; ///< Window title.
    } window;
} archi_sdl2_window_params_t;

#endif // _ARCHI_SDL2_API_WINDOW_TYP_H_

