/**
 * @file
 * @brief Application context interface for PSFv2 font.
 */

#pragma once
#ifndef _ARCHI_FONT_PSF2_CTX_FONT_VAR_H_
#define _ARCHI_FONT_PSF2_CTX_FONT_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_font_psf2_init);   ///< PSFv2 font context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_font_psf2_final); ///< PSFv2 font context finalization function.

extern
const archi_context_interface_t archi_context_font_psf2_interface; ///< PSFv2 font context interface.

#endif // _ARCHI_FONT_PSF2_CTX_FONT_VAR_H_

