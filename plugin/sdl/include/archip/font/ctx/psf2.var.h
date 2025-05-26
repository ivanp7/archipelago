/**
 * @file
 * @brief Application context interface for PSFv2 font.
 */

#pragma once
#ifndef _ARCHIP_FONT_CTX_PSF2_VAR_H_
#define _ARCHIP_FONT_CTX_PSF2_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_font_psf2_init);   ///< PSFv2 font context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_font_psf2_final); ///< PSFv2 font context finalization function.

extern
const archi_context_interface_t archip_context_font_psf2_interface; ///< PSFv2 font context interface.

#endif // _ARCHIP_FONT_CTX_PSF2_VAR_H_

