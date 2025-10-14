/**
 * @file
 * @brief Application context interface for SDL windows (OpenGL renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_WINDOW_OPENGL_VAR_H_
#define _ARCHI_SDL2_CTX_WINDOW_OPENGL_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_opengl_init);   ///< SDL window context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_opengl_final); ///< SDL window context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_sdl2_window_opengl_get);     ///< SDL window context getter function.

extern
const archi_context_interface_t archi_context_sdl2_window_opengl_interface; ///< SDL window context interface.

#endif // _ARCHI_SDL2_CTX_WINDOW_OPENGL_VAR_H_

