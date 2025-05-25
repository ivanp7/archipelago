/**
 * @file
 * @brief Operations with fonts.
 */

#pragma once
#ifndef _ARCHIP_FONT_PSF2_FUN_H_
#define _ARCHIP_FONT_PSF2_FUN_H_

#include "archip/font/psf2.typ.h"
#include "archi/util/status.typ.h"

#include <stddef.h> // for size_t

struct archip_font_psf2;

/**
 * @brief Pointer to PC Screen Font version 2.
 */
typedef struct archip_font_psf2 *archip_font_psf2_t;

/**
 * @brief Load PC Screen Font version 2 from buffer.
 *
 * @return Font.
 */
archip_font_psf2_t
archip_font_psf2_load(
        const void *bytes, ///< [in] Buffer with font data.
        size_t num_bytes,  ///< [in] Size of the buffer.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Unload PC Screen Font version 2.
 */
void
archip_font_psf2_unload(
        archip_font_psf2_t font ///< [in] Font to unload.
);

/**
 * @brief Get glyph (PC Screen Font version 2) of a character.
 *
 * @return Character glyph.
 */
const unsigned char*
archip_font_psf2_glyph(
        archip_font_psf2_t font, ///< [in] Font.

        const char *utf8_str, ///< [in] UTF-8 string.
        size_t utf8_str_len,  ///< [in] Length of UTF-8 string in bytes.

        size_t *chr_len ///< [out] Length of the first character in UTF-8 string in bytes.
);

/**
 * @brief Get PSFv2 font header.
 *
 * @return Font header.
 */
archip_font_psf2_header_t
archip_font_psf2_header(
        archip_font_psf2_t font ///< [in] Font.
);

#endif // _ARCHIP_FONT_PSF2_FUN_H_

