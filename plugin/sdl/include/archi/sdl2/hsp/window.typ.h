/**
 * @file
 * @brief Types for hierarchical state processor states for SDL2 window operations.
 */

#pragma once
#ifndef _ARCHI_SDL2_HSP_WINDOW_TYP_H_
#define _ARCHI_SDL2_HSP_WINDOW_TYP_H_

#include "archi/sdl2/api/window.fun.h"
#include "archi/mem/api/interface.fun.h"

/**
 * @brief Parameters for rendering SDL2 window area.
 */
typedef struct archi_sdl2_window_render_data {
    archi_sdl2_window_context_t window; ///< Window context.

    archi_memory_t texture; ///< Texture data to render.
    void *texture_map_data; ///< Data for texture mapping.
} archi_sdl2_window_render_data_t;

#endif // _ARCHI_SDL2_HSP_WINDOW_TYP_H_

