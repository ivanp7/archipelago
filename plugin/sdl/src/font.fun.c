/**
 * @file
 * @brief Operations with fonts.
 */

#include "font.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()

#define NUM_UNICODE_CODE_POINTS (1 + 0x10FFFF) // 0 - 0x10FFFF

static
size_t
plugin_decode_utf8_code_point(
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

plugin_font_psf2_t*
plugin_font_psf2_load_from_bytes(
        const void *bytes,
        size_t num_bytes,

        archi_status_t *code)
{
    if (bytes == NULL)
    {
        if (code != NULL)
            *code = ARCHI_ERROR_MISUSE;
        return NULL;
    }

    if (num_bytes < sizeof(plugin_font_psf2_header_t))
    {
        if (code != NULL)
            *code = ARCHI_ERROR_FORMAT;
        return NULL;
    }

    const plugin_font_psf2_header_t *header = bytes;
    if ((header->magic != PLUGIN_FONT_PSF2_MAGIC) || (header->version != 0))
    {
        if (code != NULL)
            *code = ARCHI_ERROR_FORMAT;
        return NULL;
    }

    if ((header->header_size < sizeof(plugin_font_psf2_header_t)) ||
            (header->bytes_per_glyph == 0) || (header->num_glyphs == 0))
    {
        if (code != NULL)
            *code = ARCHI_ERROR_FORMAT;
        return NULL;
    }

    if (num_bytes < header->header_size +
            (size_t)header->bytes_per_glyph * header->num_glyphs)
    {
        if (code != NULL)
            *code = ARCHI_ERROR_FORMAT;
        return NULL;
    }

    plugin_font_psf2_t *font = malloc(sizeof(*font));
    if (font == NULL)
    {
        if (code != NULL)
            *code = ARCHI_ERROR_ALLOC;
        return NULL;
    }

    font->header = (plugin_font_psf2_header_t*)header;
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
                *code = ARCHI_ERROR_ALLOC;
            return NULL;
        }

        for (uint32_t i = 0; i < NUM_UNICODE_CODE_POINTS; i++)
            font->mapping_table[i] = 0; // map all code points to glyph #0 by default

        // Decode mapping table
        uint32_t glyph_idx = 0;
        while (remaining_bytes > 0)
        {
            uint32_t code_point;
            size_t seq_len = plugin_decode_utf8_code_point(table, remaining_bytes, &code_point);

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
plugin_font_psf2_unload(
        plugin_font_psf2_t *font)
{
    if (font == NULL)
        return;

    free(font->mapping_table);
    free(font);
}

const unsigned char*
plugin_font_psf2_glyph(
        const plugin_font_psf2_t *font,

        const char *utf8_str,
        size_t utf8_str_len,

        size_t *chr_len)
{
    if ((font == NULL) || (utf8_str == NULL))
        return NULL;

    uint32_t code_point;
    size_t seq_len = plugin_decode_utf8_code_point(
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

size_t
plugin_font_psf2_glyph_data_size(
        plugin_font_psf2_header_t *header)
{
    if (header == NULL)
        return 0;

    return header->header_size + (size_t)header->bytes_per_glyph * header->num_glyphs;
}

/*****************************************************************************/

static
ARCHI_CONTAINER_ELEMENT_FUNC(plugin_font_psf2_context_init_config)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_value_t *value = element;
    archi_value_t *bytes = data;

    if (strcmp(key, PLUGIN_FONT_PSF2_CONFIG_KEY_BYTES) == 0)
    {
        if ((value->type != ARCHI_VALUE_DATA) || (value->ptr == NULL) ||
                (value->size == 0) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        *bytes = *value;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_font_psf2_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_value_t font_bytes = {0};
    if (config.data != NULL)
    {
        code = archi_container_traverse(config, plugin_font_psf2_context_init_config, &font_bytes);
        if (code != 0)
            return code;
    }

    plugin_font_psf2_t *font_context = plugin_font_psf2_load_from_bytes(font_bytes.ptr, font_bytes.size, &code);
    if (code != 0)
        return code;

    *context = font_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_font_psf2_context_final)
{
    plugin_font_psf2_unload(context);
}

const archi_context_interface_t plugin_font_psf2_context_interface = {
    .init_fn = plugin_font_psf2_context_init,
    .final_fn = plugin_font_psf2_context_final,
};

