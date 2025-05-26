/**
 * @file
 * @brief Application context interface for SDL library initialization.
 */

#pragma once
#ifndef _ARCHIP_SDL2_CTX_LIBRARY_VAR_H_
#define _ARCHIP_SDL2_CTX_LIBRARY_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_sdl2_library_init);   ///< SDL library initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_sdl2_library_final); ///< SDL library finalization function.

extern
const archi_context_interface_t archip_context_sdl2_library_interface; ///< SDL library interface.

#endif // _ARCHIP_SDL2_CTX_LIBRARY_VAR_H_

