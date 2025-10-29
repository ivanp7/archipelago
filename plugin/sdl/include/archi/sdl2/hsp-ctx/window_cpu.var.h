/**
 * @file
 * @brief Application context interface for SDL window rendering data (CPU renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_HSP_CTX_WINDOW_CPU_VAR_H_
#define _ARCHI_SDL2_HSP_CTX_WINDOW_CPU_VAR_H_

#include "archi/context/ctx/struct.fun.h"

/**
 * @brief Context initialization function for archi_sdl2_window_cpu_render_data_t.
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_cpu_render_data_init);

/**
 * @brief Context interface for archi_sdl2_window_cpu_render_data_t.
 */
extern
const archi_context_interface_t archi_context_sdl2_window_cpu_render_data_interface;

#endif // _ARCHI_SDL2_HSP_CTX_WINDOW_CPU_VAR_H_

