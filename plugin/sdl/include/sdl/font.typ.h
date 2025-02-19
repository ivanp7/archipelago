/**
 * @file
 * @brief Types for the font support.
 */

#pragma once
#ifndef _PLUGIN_SDL_FONT_TYP_H_
#define _PLUGIN_SDL_FONT_TYP_H_

#include <stdint.h> // for uint32_t

/**
 * @brief Magic constant of PC Screen Font version 2.
 */
#define PLUGIN_FONT_PSF2_MAGIC 0x864ab572

/**
 * @brief Header of PC Screen Font version 2.
 */
typedef struct plugin_font_psf2_header {
    uint32_t magic;           ///< Magic bytes to identify PSF.
    uint32_t version;         ///< Zero.
    uint32_t header_size;     ///< Offset of bitmaps in file, 32.
    uint32_t flags;           ///< 1 if there's Unicode table, 0 otherwise.
    uint32_t num_glyphs;      ///< Number of glyphs.
    uint32_t bytes_per_glyph; ///< Size of each glyph.
    uint32_t height;          ///< Height in pixels.
    uint32_t width;           ///< Width in pixels.
} plugin_font_psf2_header_t;

/**
 * @brief PC Screen Font version 2, representation in memory.
 */
typedef struct plugin_font_psf2 {
    plugin_font_psf2_header_t *header; ///< Font header.
    unsigned char *glyphs;             ///< Font glyphs.

    uint32_t *mapping_table; ///< (Unicode code point) -> (glyph index) mapping table.
} plugin_font_psf2_t;

#endif // _PLUGIN_SDL_FONT_TYP_H_

