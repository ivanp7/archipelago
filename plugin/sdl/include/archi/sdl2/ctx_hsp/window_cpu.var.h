/**
 * @file
 * @brief Application context interface for SDL window rendering data (CPU renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_HSP_WINDOW_CPU_VAR_H_
#define _ARCHI_SDL2_CTX_HSP_WINDOW_CPU_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_cpu_render_data_init);   ///< SDL window rendering data initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_cpu_render_data_final); ///< SDL window rendering data finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_sdl2_window_cpu_render_data_get);     ///< SDL window rendering data getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_sdl2_window_cpu_render_data_set);     ///< SDL window rendering data setter function.

extern
const archi_context_interface_t archi_context_sdl2_window_cpu_render_data_interface; ///< SDL window rendering data interface.

#endif // _ARCHI_SDL2_CTX_HSP_WINDOW_CPU_VAR_H_

