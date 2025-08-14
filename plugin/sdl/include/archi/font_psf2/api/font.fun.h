/**
 * @file
 * @brief Operations with fonts.
 */

#pragma once
#ifndef _ARCHI_FONT_PSF2_API_FONT_FUN_H_
#define _ARCHI_FONT_PSF2_API_FONT_FUN_H_

#include "archi/font_psf2/api/font.typ.h"
#include "archipelago/base/status.typ.h"

#include <stddef.h> // for size_t

struct archi_font_psf2;

/**
 * @brief Pointer to PC Screen Font version 2.
 */
typedef struct archi_font_psf2 *archi_font_psf2_t;

/**
 * @brief Load PC Screen Font version 2 from buffer.
 *
 * @return Font.
 */
archi_font_psf2_t
archi_font_psf2_load(
        const void *bytes, ///< [in] Buffer with font data.
        size_t num_bytes,  ///< [in] Size of the buffer.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Unload PC Screen Font version 2.
 */
void
archi_font_psf2_unload(
        archi_font_psf2_t font ///< [in] Font to unload.
);

/**
 * @brief Get glyph (PC Screen Font version 2) of a character.
 *
 * @return Character glyph.
 */
const unsigned char*
archi_font_psf2_glyph(
        archi_font_psf2_t font, ///< [in] Font.

        const char *utf8_str, ///< [in] UTF-8 string.
        size_t utf8_str_len,  ///< [in] Length of UTF-8 string in bytes.

        size_t *chr_len ///< [out] Length of the first character in UTF-8 string in bytes.
);

/**
 * @brief Get PSFv2 font header.
 *
 * @return Font header.
 */
archi_font_psf2_header_t
archi_font_psf2_header(
        archi_font_psf2_t font ///< [in] Font.
);

#endif // _ARCHI_FONT_PSF2_API_FONT_FUN_H_

