/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Operations with PSFv2 fonts.
 */

#include "archi/font_psf2/api/font.fun.h"
#include "archi/font_psf2/api/font.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()
#include <limits.h> // for CHAR_BIT


#define NUM_UNICODE_CODE_POINTS (ARCHI_STRING_UNICODE_CODEPOINT_MAX + 1)

/**
 * @brief PC Screen Font version 2, representation in memory.
 */
struct archi_font_psf2 {
    archi_rcpointer_t data; ///< Font data.
    archi_font_psf2_header_t header; ///< Font header.
    uint32_t *mapping_table; ///< (Unicode code point) -> (glyph index) mapping table.
};

archi_font_psf2_t
archi_font_psf2_load(
        archi_rcpointer_t font_data,
        ARCHI_ERROR_PARAM_DECL)
{
    // Check input pointer
    if (!archi_pointer_attr_compatible(font_data.attr,
                ARCHI_POINTER_ATTR__PDATA(sizeof(archi_font_psf2_header_t), char)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "font data pointer has incorrect attributes");
        return NULL;
    }
    else if (font_data.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "font data pointer is NULL");
        return NULL;
    }

    size_t font_data_size;
    {
        size_t length, stride;
        archi_pointer_attr_unpk__pdata(font_data.attr, &length, &stride, NULL, NULL);
        font_data_size = length * stride;
    }

    // Allocate the font object
    archi_font_psf2_t font = malloc(sizeof(*font));
    if (font == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "couldn't allocate font object");
        return NULL;
    }

    // Initialize the font object
    memcpy(&font->header, font_data.ptr, sizeof(archi_font_psf2_header_t));
    font->mapping_table = NULL;

    // Check the font header
    if (font->header.magic != ARCHI_FONT_PSF2_MAGIC)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "magic number in font header is incorrect");
        goto failure;
    }
    else if (font->header.version != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "version number in font header is incorrect");
        goto failure;
    }
    else if (font->header.header_size < sizeof(archi_font_psf2_header_t))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "header size in font header is incorrect");
        goto failure;
    }
    else if (font->header.flags > 1)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "flags in font header are incorrect");
        goto failure;
    }
    else if (font->header.num_glyphs == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of glyphs in font header is zero");
        goto failure;
    }
    else if (font->header.bytes_per_glyph == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of bytes per glyph in font header is zero");
        goto failure;
    }
    else if (font_data_size < font->header.header_size +
            (size_t)font->header.bytes_per_glyph * font->header.num_glyphs)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data size is not enough to fit all font glyphs");
        goto failure;
    }
    else if (font->header.height == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "glyph height in font header is zero");
        goto failure;
    }
    else if (font->header.width == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "glyph width in font header is zero");
        goto failure;
    }

    // Generate the mapping table
    if (font->header.flags)
    {
        unsigned char *table = (unsigned char*)font_data.ptr +
            font->header.header_size + (size_t)font->header.bytes_per_glyph * font->header.num_glyphs;
        unsigned char *table_end = (unsigned char*)font_data.ptr + font_data_size;

        size_t remaining_bytes = table_end - table;

        font->mapping_table = malloc(sizeof(*font->mapping_table) * NUM_UNICODE_CODE_POINTS);
        if (font->mapping_table == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate font mapping table");
            goto failure;
        }

        for (uint32_t i = 0; i < NUM_UNICODE_CODE_POINTS; i++)
            font->mapping_table[i] = 0; // map all code points to glyph #0 by default

        // Decode mapping table
        uint32_t glyph_idx = 0;
        while (remaining_bytes > 0)
        {
            size_t seq_len;

            if (table[0] == 0xFF) // record end
            {
                glyph_idx++;
                seq_len = 1;
            }
            else
            {
                uint32_t code_point = archi_string_decode_utf8((char*)table, remaining_bytes, &seq_len);
                if (code_point != ARCHI_STRING_UNICODE_CODEPOINT_REPLACEMENT) // valid code point
                    font->mapping_table[code_point] = glyph_idx;
            }

            table += seq_len;
            remaining_bytes -= seq_len;
        }
    }

    // Increment the font data reference counter
    font->data = archi_rcpointer_own(font_data, ARCHI_ERROR_PARAM);
    if (!font->data.attr)
        goto failure;

    ARCHI_ERROR_RESET();
    return font;

failure:
    free(font);

    return NULL;
}

void
archi_font_psf2_unload(
        archi_font_psf2_t font)
{
    if (font == NULL)
        return;

    archi_rcpointer_disown(font->data);
    free(font->mapping_table);
    free(font);
}

archi_font_psf2_header_t
archi_font_psf2_header(
        archi_font_psf2_t font)
{
    if (font == NULL)
        return (archi_font_psf2_header_t){0};

    return font->header;
}

archi_rcpointer_t
archi_font_psf2_glyph(
        archi_font_psf2_t font,
        uint32_t code_point)
{
    if (font == NULL)
        return (archi_rcpointer_t){0};
    else if (code_point > ARCHI_STRING_UNICODE_CODEPOINT_MAX)
        return (archi_rcpointer_t){0};

    uint32_t glyph_idx = (font->mapping_table != NULL) ? font->mapping_table[code_point] : code_point;
    if (glyph_idx >= font->header.num_glyphs)
        return (archi_rcpointer_t){0};

    return (archi_rcpointer_t){
        .ptr = (unsigned char*)font->data.ptr + font->header.header_size +
            (size_t)font->header.bytes_per_glyph * glyph_idx,
        .attr = (font->data.attr & ARCHI_POINTER_TYPE_MASK) |
            ARCHI_POINTER_ATTR__PDATA(font->header.bytes_per_glyph, char),
        .ref_count = font->data.ref_count,
    };
}

/*****************************************************************************/

bool
archi_font_psf2_draw_glyph(
        const unsigned char *glyph,
        uint32_t glyph_width,
        uint32_t glyph_height,

        uint32_t left_col_idx,
        uint32_t top_row_idx,
        uint32_t right_col_idx,
        uint32_t bottom_row_idx,

        unsigned char *buffer,
        size_t buffer_pitch,
        size_t buffer_width,
        size_t buffer_height,

        size_t x,
        size_t y,

        size_t pixel_size,
        const unsigned char *fg_pixel,
        const unsigned char *bg_pixel)
{
    // Check inputs for correctness
    if (glyph == NULL)
        return false;
    else if ((glyph_width == 0) || (glyph_height == 0))
        return false;
    else if ((left_col_idx >= glyph_width) || (right_col_idx >= glyph_width))
        return false;
    else if ((top_row_idx >= glyph_height) || (bottom_row_idx >= glyph_height))
        return false;
    else if (buffer == NULL)
        return false;
    else if ((buffer_width == 0) || (buffer_height == 0))
        return false;
    else if (buffer_pitch < buffer_width)
        return false;
    else if ((x >= buffer_width) || (y >= buffer_height))
        return false;
    else if (pixel_size == 0)
        return false;
    else if ((fg_pixel == NULL) && (bg_pixel == NULL))
        return false;
    else
    {
        uint32_t num_cols = 1 + (left_col_idx <= right_col_idx ?
                right_col_idx - left_col_idx : left_col_idx - right_col_idx);
        uint32_t num_rows = 1 + (top_row_idx <= bottom_row_idx ?
                bottom_row_idx - top_row_idx : top_row_idx - bottom_row_idx);

        if (x + num_cols > buffer_width)
            return false;
        else if (y + num_rows > buffer_height)
            return false;
    }

    // Prepare auxiliary values
    size_t bytes_per_row = (glyph_width + (CHAR_BIT - 1)) / CHAR_BIT;

    int32_t col_idx_delta = left_col_idx <= right_col_idx ? 1 : -1;
    int32_t row_idx_delta = top_row_idx <= bottom_row_idx ? 1 : -1;

    // Draw glyph rows
    uint32_t row_idx = top_row_idx;
    for (size_t i = 0;; i++)
    {
        const unsigned char *glyph_row = glyph + bytes_per_row * row_idx;
        unsigned char *buffer_row = buffer + pixel_size * buffer_pitch * (y + i);

        // Draw glyph columns
        uint32_t col_idx = left_col_idx;
        for (size_t j = 0;; j++)
        {
            unsigned char *buffer_row_col = buffer_row + pixel_size * (x + j);

            // Check if the current pixel is foreground
            bool pixel_is_fg = glyph_row[col_idx / CHAR_BIT] &
                (1 << ((CHAR_BIT - 1) - (col_idx % CHAR_BIT)));

            // Update the buffer
            if (pixel_is_fg && (fg_pixel != NULL))
                memcpy(buffer_row_col, fg_pixel, pixel_size);
            else if (!pixel_is_fg && (bg_pixel != NULL))
                memcpy(buffer_row_col, bg_pixel, pixel_size);

            // Go to the next column or stop
            if (col_idx != right_col_idx)
                col_idx += col_idx_delta;
            else
                break;
        }

        // Go to the next row or stop
        if (row_idx != bottom_row_idx)
            row_idx += row_idx_delta;
        else
            break;
    }

    return true;
}

