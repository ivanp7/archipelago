/**
 * @file
 * @brief Operations with fonts.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_SDL_FONT_FUN_H_
#define _ARCHI_PLUGIN_SDL_FONT_FUN_H_

#include "archi/plugin/sdl/font.typ.h"
#include "archi/util/status.typ.h"

#include <stddef.h>

/**
 * @brief Load PC Screen Font version 2 from buffer.
 *
 * @return Font.
 */
archip_font_psf2_t*
archip_font_psf2_load_from_bytes(
        const void *bytes, ///< [in] Buffer with font data.
        size_t num_bytes,  ///< [in] Size of the buffer.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Unload PC Screen Font version 2.
 */
void
archip_font_psf2_unload(
        archip_font_psf2_t *font ///< [in] Font to unload.
);

/**
 * @brief Get glyph (PC Screen Font version 2) of a character.
 *
 * @return Character glyph.
 */
const unsigned char*
archip_font_psf2_glyph(
        const archip_font_psf2_t *font, ///< [in] Font.

        const char *utf8_str, ///< [in] UTF-8 string.
        size_t utf8_str_len,  ///< [in] Length of UTF-8 string in bytes.

        size_t *chr_len ///< [out] Length of the first character in UTF-8 string in bytes.
);

/**
 * @brief Get font data size -- full font size minus Unicode table.
 *
 * This function can be used to resize font buffer after loading the font
 * to release Unicode table that is no longer needed.
 *
 * @return Font data size.
 */
size_t
archip_font_psf2_glyph_data_size(
        archip_font_psf2_header_t *header ///< [in] Font header.
);

#endif // _ARCHI_PLUGIN_SDL_FONT_FUN_H_

