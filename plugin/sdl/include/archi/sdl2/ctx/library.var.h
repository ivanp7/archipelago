/**
 * @file
 * @brief Application context interface for SDL library initialization.
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_LIBRARY_VAR_H_
#define _ARCHI_SDL2_CTX_LIBRARY_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_library_init);   ///< SDL library initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_library_final); ///< SDL library finalization function.

extern
const archi_context_interface_t archi_context_sdl2_library_interface; ///< SDL library interface.

#endif // _ARCHI_SDL2_CTX_LIBRARY_VAR_H_

