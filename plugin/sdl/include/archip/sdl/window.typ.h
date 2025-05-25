/**
 * @file
 * @brief Types for operations with SDL windows.
 */

#pragma once
#ifndef _ARCHIP_SDL_WINDOW_TYP_H_
#define _ARCHIP_SDL_WINDOW_TYP_H_

#include <stdint.h> // for uint32_t

/**
 * @brief Properties of a created SDL window.
 *
 * If window width is 0, it is substituted with texture width.
 * If window height is 0, it is substituted with texture height.
 */
typedef struct archip_sdl_window_config {
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
} archip_sdl_window_config_t;

#endif // _ARCHIP_SDL_WINDOW_TYP_H_

