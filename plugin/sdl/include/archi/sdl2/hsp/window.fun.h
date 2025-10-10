/**
 * @file
 * @brief Hierarchical state processor states for SDL2 window operations.
 */

#pragma once
#ifndef _ARCHI_SDL2_HSP_WINDOW_FUN_H_
#define _ARCHI_SDL2_HSP_WINDOW_FUN_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief State function for rendering SDL2 window area.
 *
 * This state function expects archi_sdl2_window_render_data_t
 * object as function data.
 */
ARCHI_HSP_STATE_FUNCTION(archi_sdl2_hsp_state_window_render);

#endif // _ARCHI_SDL2_HSP_WINDOW_FUN_H_

