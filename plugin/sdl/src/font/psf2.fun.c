/**
 * @file
 * @brief Operations with fonts.
 */

#include "archip/font/psf2.fun.h"

#include <stdlib.h> // for malloc(), free()

#define NUM_UNICODE_CODE_POINTS (0x10FFFF + 1) // 0 - 0x10FFFF

/**
 * @brief PC Screen Font version 2, representation in memory.
 */
struct archip_font_psf2 {
    archip_font_psf2_header_t *header; ///< Font header.
    unsigned char *glyphs;             ///< Font glyphs.

    uint32_t *mapping_table; ///< (Unicode code point) -> (glyph index) mapping table.
};

static
size_t
archip_decode_utf8_code_point(
        const unsigned char *seq,
        size_t remaining_bytes,
        uint32_t *code_point)
{
    *code_point = -1; // invalid code point signifies error

    if (remaining_bytes == 0)
        return 0;

    unsigned char byte1 = seq[0];

    if ((byte1 & 0xF8) == 0xF8) // invalid first byte
    {
        if (byte1 == 0xFF) // record separator byte
            *code_point = NUM_UNICODE_CODE_POINTS;
        return 1;
    }

    if (byte1 & 0x80) // Unicode
    {
        if (byte1 & 0x40) // valid first byte
        {
            if (remaining_bytes < 2)
                return 1;

            unsigned char byte2 = seq[1];
            if ((byte2 & 0xC0) != 0x80)
                return 1;

            if (byte1 & 0x20) // 3-4 bytes
            {
                if (remaining_bytes < 3)
                    return 2;

                unsigned char byte3 = seq[2];
                if ((byte3 & 0xC0) != 0x80)
                    return 2;

                if (byte1 & 0x10) // 4 bytes
                {
                    if (remaining_bytes < 4)
                        return 3;

                    unsigned char byte4 = seq[3];
                    if ((byte4 & 0xC0) != 0x80)
                        return 3;

                    *code_point = byte1 & 0x07;
                    *code_point = (*code_point << 6) | (byte2 & 0x3F);
                    *code_point = (*code_point << 6) | (byte3 & 0x3F);
                    *code_point = (*code_point << 6) | (byte4 & 0x3F);
                    return 4;
                }
                else // 3 bytes
                {
                    *code_point = byte1 & 0x0F;
                    *code_point = (*code_point << 6) | (byte2 & 0x3F);
                    *code_point = (*code_point << 6) | (byte3 & 0x3F);
                    return 3;
                }
            }
            else // 2 bytes
            {
                *code_point = byte1 & 0x1F;
                *code_point = (*code_point << 6) | (byte2 & 0x3F);
                return 2;
            }
        }
        else // invalid first byte
        {
            // Skip until valid first byte
            size_t skip = 1;
            while ((skip < remaining_bytes) && ((seq[skip] & 0xC0) == 0x80))
                skip++;

            return skip;
        }
    }
    else // ASCII
    {
        *code_point = byte1;
        return 1;
    }
}

archip_font_psf2_t
archip_font_psf2_load(
        const void *bytes,
        size_t num_bytes,

        archi_status_t *code)
{
    if (bytes == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;
        return NULL;
    }

    if (num_bytes < sizeof(archip_font_psf2_header_t))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EVALUE;
        return NULL;
    }

    const archip_font_psf2_header_t *header = bytes;
    if ((header->magic != ARCHIP_FONT_PSF2_MAGIC) || (header->version != 0))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EVALUE;
        return NULL;
    }

    if ((header->header_size < sizeof(archip_font_psf2_header_t)) ||
            (header->bytes_per_glyph == 0) || (header->num_glyphs == 0))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EVALUE;
        return NULL;
    }

    if (num_bytes < header->header_size +
            (size_t)header->bytes_per_glyph * header->num_glyphs)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EVALUE;
        return NULL;
    }

    archip_font_psf2_t font = malloc(sizeof(*font));
    if (font == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;
        return NULL;
    }

    font->header = (archip_font_psf2_header_t*)header;
    font->glyphs = (unsigned char*)bytes + header->header_size;

    if (!header->flags)
        font->mapping_table = NULL;
    else
    {
        unsigned char *table_end = (unsigned char*)bytes + num_bytes;
        unsigned char *table = font->glyphs +
            (size_t)header->bytes_per_glyph * header->num_glyphs;

        size_t remaining_bytes = table_end - table;

        font->mapping_table = malloc(sizeof(*font->mapping_table) * NUM_UNICODE_CODE_POINTS);
        if (font->mapping_table == NULL)
        {
            free(font);

            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;
            return NULL;
        }

        for (uint32_t i = 0; i < NUM_UNICODE_CODE_POINTS; i++)
            font->mapping_table[i] = 0; // map all code points to glyph #0 by default

        // Decode mapping table
        uint32_t glyph_idx = 0;
        while (remaining_bytes > 0)
        {
            uint32_t code_point;
            size_t seq_len = archip_decode_utf8_code_point(table, remaining_bytes, &code_point);

            if (code_point < NUM_UNICODE_CODE_POINTS) // valid code point
                font->mapping_table[code_point] = glyph_idx;
            else if (code_point == NUM_UNICODE_CODE_POINTS) // record end
                glyph_idx++;

            table += seq_len;
            remaining_bytes -= seq_len;
        }
    }

    if (code != NULL)
        *code = 0;

    return font;
}

void
archip_font_psf2_unload(
        archip_font_psf2_t font)
{
    if (font == NULL)
        return;

    free(font->mapping_table);
    free(font);
}

const unsigned char*
archip_font_psf2_glyph(
        archip_font_psf2_t font,

        const char *utf8_str,
        size_t utf8_str_len,

        size_t *chr_len)
{
    if ((font == NULL) || (utf8_str == NULL))
        return NULL;

    uint32_t code_point;
    size_t seq_len = archip_decode_utf8_code_point(
            (const unsigned char*)utf8_str, utf8_str_len, &code_point);

    if (code_point >= NUM_UNICODE_CODE_POINTS) // invalid code point
        return NULL;

    if (chr_len != NULL)
        *chr_len = seq_len;

    uint32_t glyph_idx = (font->mapping_table == NULL) ?
        code_point : font->mapping_table[code_point];

    if (glyph_idx >= font->header->num_glyphs)
        return NULL;

    return font->glyphs + (size_t)font->header->bytes_per_glyph * glyph_idx;
}

archip_font_psf2_header_t
archip_font_psf2_header(
        archip_font_psf2_t font)
{
    if ((font == NULL) || (font->header == NULL))
        return (archip_font_psf2_header_t){0};

    return *font->header;
}

