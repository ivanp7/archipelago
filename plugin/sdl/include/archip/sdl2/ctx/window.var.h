/**
 * @file
 * @brief Application context interface for SDL windows.
 */

#pragma once
#ifndef _ARCHIP_SDL2_CTX_WINDOW_VAR_H_
#define _ARCHIP_SDL2_CTX_WINDOW_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_sdl2_window_init);   ///< SDL window context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_sdl2_window_final); ///< SDL window context finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_sdl2_window_get);     ///< SDL window context getter function.

extern
const archi_context_interface_t archip_context_sdl2_window_interface; ///< SDL window context interface.

#endif // _ARCHIP_SDL2_CTX_WINDOW_VAR_H_

