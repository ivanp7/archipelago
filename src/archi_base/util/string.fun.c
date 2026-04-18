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
 * @brief Operations on strings.
 */

#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc()
#include <string.h> // for strlen(), memcpy()


size_t
archi_string_hash(
        const char *string)
{
    if (string == NULL)
        return 0;

    size_t hash = 5381;

    char c;
    while ((c = *string++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash;
}

char*
archi_string_copy(
        const char *string)
{
    if (string == NULL)
        return NULL;

    size_t string_size = strlen(string) + 1;

    char *string_copy = malloc(string_size);
    if (string_copy == NULL)
        return NULL;

    memcpy(string_copy, string, string_size);

    return string_copy;
}

char*
archi_string_concat(
        const char *strings[],
        size_t num_strings,
        const char *separator)
{
    if ((num_strings != 0) && (strings == NULL))
        return NULL;

    if (separator == NULL)
        separator = "";

    size_t sep_length = strlen(separator);

    // Compute the resulting string length
    size_t total_length = 0;

    for (size_t i = 0; i < num_strings; i++)
    {
        if (i > 0)
            total_length += sep_length;

        if (strings[i] != NULL)
            total_length += strlen(strings[i]);
    }

    // Allocate memory for the resulting string
    char *result = malloc(total_length + 1);
    if (result == NULL)
        return NULL;

    // Construct the resulting string
    size_t position = 0;

    for (size_t i = 0; i < num_strings; i++)
    {
        if (i > 0)
        {
            memcpy(result + position, separator, sep_length);
            position += sep_length;
        }

        if (strings[i] != NULL)
        {
            size_t length = strlen(strings[i]);
            memcpy(result + position, strings[i], length);
            position += length;
        }
    }

    // Write null-terminator to the end
    result[total_length] = '\0';

    return result;
}

/*****************************************************************************/

uint32_t
archi_string_decode_utf8(
        const char *string,
        size_t string_length,
        size_t *char_length)
{
    size_t length = 0;

    if ((string == NULL) || (string_length == 0))
        goto invalid;

    unsigned char c = (unsigned char)string[0];
    uint32_t code_point = 0;

    length = 1;

    if (c < 0x80) // 1-byte sequence: 0xxxxxxx
        code_point = c;
    else if (c < 0xC0) // continuation byte
    {
        // Skip until the first non-continuation byte
        while ((length < 3) && (length < string_length) && ((string[length] & 0xC0) == 0x80))
            length++;
        goto invalid;
    }
    else if (c < 0xE0) // 2-byte sequence: 110xxxxx 10xxxxxx
    {
        if (string_length < 2)
        {
            length = string_length;
            goto invalid;
        }

        code_point = (c & 0x1F) << 6;

        c = (unsigned char)string[1];
        if ((c & 0xC0) != 0x80) // not a continuation byte
            goto invalid;
        else
            length = 2;

        code_point |= c & 0x3F;

        if (code_point < 0x80) // overlong encoding
            goto invalid;
    }
    else if (c < 0xF0) // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
    {
        if (string_length < 3)
        {
            length = string_length;
            goto invalid;
        }

        code_point = (c & 0x0F) << 12;

        c = (unsigned char)string[1];
        if ((c & 0xC0) != 0x80) // not a continuation byte
            goto invalid;
        else
            length = 2;

        code_point |= (c & 0x3F) << 6;

        c = (unsigned char)string[2];
        if ((c & 0xC0) != 0x80) // not a continuation byte
            goto invalid;
        else
            length = 3;

        code_point |= c & 0x3F;

        if (code_point < 0x800) // overlong encoding
            goto invalid;
        else if ((code_point >= 0xD800) && (code_point <= 0xDFFF)) // surrogates
            goto invalid;
    }
    else if (c < 0xF8) // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    {
        if (string_length < 4)
        {
            length = string_length;
            goto invalid;
        }

        code_point = (c & 0x07) << 18;

        c = (unsigned char)string[1];
        if ((c & 0xC0) != 0x80) // not a continuation byte
            goto invalid;
        else
            length = 2;

        code_point |= (c & 0x3F) << 12;

        c = (unsigned char)string[2];
        if ((c & 0xC0) != 0x80) // not a continuation byte
            goto invalid;
        else
            length = 3;

        code_point |= (c & 0x3F) << 6;

        c = (unsigned char)string[3];
        if ((c & 0xC0) != 0x80) // not a continuation byte
            goto invalid;
        else
            length = 4;

        code_point |= c & 0x3F;

        if (code_point < 0x10000) // overlong encoding
            goto invalid;
        else if (code_point > ARCHI_STRING_UNICODE_CODEPOINT_MAX) // code point out of range
            goto invalid;
    }
    else // invalid byte
        goto invalid;

    if (char_length != NULL)
        *char_length = length;

    return code_point;

invalid: // Invalid UTF-8 sequence
    if (char_length != NULL)
        *char_length = length;

    return ARCHI_STRING_UNICODE_CODEPOINT_REPLACEMENT;
}

size_t
archi_string_encode_utf8(
        char *string,
        uint32_t code_point)
{
    if (code_point > ARCHI_STRING_UNICODE_CODEPOINT_MAX) // code point out of range
        return 0;
    else if ((code_point >= 0xD800) && (code_point <= 0xDFFF)) // surrogates
        return 0;

    if (code_point < 0x80) // 1-byte sequence
    {
        if (string != NULL)
            string[0] = code_point;

        return 1;
    }
    else if (code_point < 0x800) // 2-byte sequence
    {
        if (string != NULL)
        {
            string[0] = 0xC0 | (code_point >> 6);
            string[1] = 0x80 | (code_point & 0x3F);
        }

        return 2;
    }
    else if (code_point < 0x10000) // 3-byte sequence
    {
        if (string != NULL)
        {
            string[0] = 0xE0 | (code_point >> 12);
            string[1] = 0x80 | ((code_point >> 6) & 0x3F);
            string[2] = 0x80 | (code_point & 0x3F);
        }

        return 3;
    }
    else // 4-byte sequence
    {
        if (string != NULL)
        {
            string[0] = 0xF0 | (code_point >> 18);
            string[1] = 0x80 | ((code_point >> 12) & 0x3F);
            string[2] = 0x80 | ((code_point >> 6) & 0x3F);
            string[3] = 0x80 | (code_point & 0x3F);
        }

        return 4;
    }
}

