/**
 * @file
 * @brief Application context interface for PSFv2 font.
 */

#pragma once
#ifndef _ARCHI_FONT_PSF2_CTX_FONT_VAR_H_
#define _ARCHI_FONT_PSF2_CTX_FONT_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief PSFv2 font context initialization function.
 *
 * Accepts the following parameters:
 * - "data" : font data
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_font_psf2_init);

/**
 * @brief PSFv2 font context finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_font_psf2_final);

/**
 * @brief PSFv2 font context interface.
 */
extern
const archi_context_interface_t archi_context_font_psf2_interface;

#endif // _ARCHI_FONT_PSF2_CTX_FONT_VAR_H_

