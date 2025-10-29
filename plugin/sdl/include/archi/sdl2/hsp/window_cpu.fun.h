/**
 * @file
 * @brief Hierarchical state processor states for operations with SDL2 windows (CPU renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_HSP_WINDOW_CPU_FUN_H_
#define _ARCHI_SDL2_HSP_WINDOW_CPU_FUN_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief Render SDL2 window by copying the whole texture from a memory object.
 *
 * This state function expects archi_sdl2_window_cpu_render_data_t
 * object as function data.
 */
ARCHI_HSP_STATE_FUNCTION(archi_sdl2_hsp_state_window_cpu_render);

#endif // _ARCHI_SDL2_HSP_WINDOW_CPU_FUN_H_

