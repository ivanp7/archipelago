/**
 * @file
 * @brief Operations with fonts.
 */

#pragma once
#ifndef _ARCHI_FONT_PSF2_API_FONT_FUN_H_
#define _ARCHI_FONT_PSF2_API_FONT_FUN_H_

#include "archi/font_psf2/api/font.typ.h"
#include "archipelago/base/pointer.typ.h"
#include "archipelago/base/error.typ.h"

#include <stdbool.h>

/**
 * @brief Load PC Screen Font version 2 from buffer.
 *
 * @return Font.
 */
archi_font_psf2_t
archi_font_psf2_load(
        archi_pointer_t font_data, ///< [in] Font data.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Unload PC Screen Font version 2.
 */
void
archi_font_psf2_unload(
        archi_font_psf2_t font ///< [in] Font to unload.
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

/**
 * @brief Get glyph (PC Screen Font version 2) of the first character of a string.
 *
 * @return Pointer to the character glyph.
 */
archi_pointer_t
archi_font_psf2_glyph(
        archi_font_psf2_t font, ///< [in] Font.
        uint32_t code_point ///< [in] Unicode code point.
);

/*****************************************************************************/

/**
 * @brief Draw a glyph onto a pixel buffer area.
 *
 * @return True if the buffer has been modified, otherwise false.
 */
bool
archi_font_psf2_draw_glyph(
        const unsigned char *glyph, ///< [in] Glyph to draw.
        uint32_t glyph_width,  ///< [in] Glyph width in pixels.
        uint32_t glyph_height, ///< [in] Glyph height in pixels.

        uint32_t left_col_idx,  ///< [in] Index of the glyph column to be drawn at the left side.
        uint32_t top_row_idx,    ///< [in] Index of the glyph row to be drawn at the top.
        uint32_t right_col_idx, ///< [in] Index of the glyph column to be drawn at the right side.
        uint32_t bottom_row_idx, ///< [in] Index of the glyph row to be drawn at the bottom.

        unsigned char *buffer, ///< [in] Pixel buffer area.
        size_t buffer_pitch,   ///< [in] Area pitch in pixels.
        size_t buffer_width,   ///< [in] Area width in pixels.
        size_t buffer_height,  ///< [in] Area height in pixels.

        size_t x, ///< [in] Horizontal displacement of upper left corner of the drawn glyph.
        size_t y, ///< [in] Vertical displacement of upper left corner of the drawn glyph.

        size_t pixel_size, ///< [in] Size of a pixel in bytes.
        const unsigned char *fg_pixel, ///< [in] Glyph foreground color.
        const unsigned char *bg_pixel  ///< [in] Glyph background color.
);

#endif // _ARCHI_FONT_PSF2_API_FONT_FUN_H_

