/**
 * @file
 * @brief Types for operations with SDL windows (CPU renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_API_WINDOW_CPU_TYP_H_
#define _ARCHI_SDL2_API_WINDOW_CPU_TYP_H_

#include "archi/sdl2/api/window.typ.h"

struct archi_sdl2_window_cpu_context;

/**
 * @brief Pointer to SDL window context (CPU renderer).
 */
typedef struct archi_sdl2_window_cpu_context *archi_sdl2_window_cpu_context_t;

/**
 * @brief Parameters for archi_sdl2_window_cpu_create().
 *
 * If window width is 0, it is substituted with texture width.
 * If window height is 0, it is substituted with texture height.
 */
typedef struct archi_sdl2_window_cpu_params {
    archi_sdl2_window_params_t window; ///< Window parameters.

    struct {
        int width;  ///< Texture width in pixels.
        int height; ///< Texture height in pixels.
    } texture;
} archi_sdl2_window_cpu_params_t;

/**
 * @brief Texture pixel color.
 */
typedef uint32_t archi_sdl2_pixel_t;

#endif // _ARCHI_SDL2_API_WINDOW_CPU_TYP_H_

