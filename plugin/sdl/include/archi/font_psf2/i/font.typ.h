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
 * @brief Types for the PSFv2 font support.
 */

#pragma once
#ifndef _ARCHI_FONT_PSF2_API_FONT_TYP_H_
#define _ARCHI_FONT_PSF2_API_FONT_TYP_H_

#include <stdint.h> // for uint32_t


/**
 * @brief Header of PC Screen Font version 2.
 */
typedef struct archi_font_psf2_header {
    uint32_t magic;           ///< Magic bytes to identify PSF.
    uint32_t version;         ///< Zero.
    uint32_t header_size;     ///< Offset of bitmaps in file, 32.
    uint32_t flags;           ///< 1 if there's Unicode table, 0 otherwise.
    uint32_t num_glyphs;      ///< Number of glyphs.
    uint32_t bytes_per_glyph; ///< Size of each glyph.
    uint32_t height;          ///< Height in pixels.
    uint32_t width;           ///< Width in pixels.
} archi_font_psf2_header_t;

/**
 * @brief Calculate PSFv2 font data size -- full font size minus Unicode table.
 *
 * Font buffer may be resized to this size after loading the font
 * in order to release Unicode table that is no longer needed.
 */
#define ARCHI_FONT_PSF2_SIZE(header)   \
    ((header).header_size + (size_t)(header).bytes_per_glyph * (header).num_glyphs)

#endif // _ARCHI_FONT_PSF2_API_FONT_TYP_H_

