/**
 * @file
 * @brief Types for hierarchical state processor states for operations on SDL2 windows (CPU renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_HSP_WINDOW_CPU_TYP_H_
#define _ARCHI_SDL2_HSP_WINDOW_CPU_TYP_H_

#include "archi/sdl2/api/window_cpu.fun.h"
#include "archi/mem/api/interface.fun.h"

/**
 * @brief Parameters for rendering SDL2 window (CPU renderer).
 */
typedef struct archi_sdl2_window_cpu_render_data {
    archi_sdl2_window_cpu_context_t window; ///< Window context.

    archi_memory_t texture; ///< Texture data to render.
    void *texture_map_data; ///< Data for texture mapping.
} archi_sdl2_window_cpu_render_data_t;

#endif // _ARCHI_SDL2_HSP_WINDOW_CPU_TYP_H_

