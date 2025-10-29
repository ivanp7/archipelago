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

#pragma once
#ifndef _ARCHIPELAGO_UTIL_STRING_FUN_H_
#define _ARCHIPELAGO_UTIL_STRING_FUN_H_

#include <string.h> // for strcmp()
#include <stddef.h> // for size_t
#include <stdint.h> // for uint32_t

/**
 * @brief Compare strings for equality or order.
 */
#define ARCHI_STRING_COMPARE(str1, op, str2)    \
    (strcmp((str1), (str2)) op 0)

/**
 * @brief Allocate a string copy.
 *
 * @return String copy.
 */
char*
archi_string_copy(
        const char *string ///< [in] Original string.
);

/**
 * @brief Compute hash of a string.
 *
 * This function implements the djb2 algorithm.
 *
 * @return Hash value.
 */
size_t
archi_string_hash(
        const char *string ///< [in] String.
);

/*****************************************************************************/

/**
 * @brief The largest possible Unicode code point.
 */
#define ARCHI_STRING_UNICODE_CODEPOINT_MAX          0x10FFFF

/**
 * @brief The Unicode code point used to replace an unknown, unrecognised, or unrepresentable character.
 */
#define ARCHI_STRING_UNICODE_CODEPOINT_REPLACEMENT  0xFFFD

/**
 * @brief Decode the first character of a UTF-8 string, computing its length in bytes.
 *
 * The function checks for valid UTF-8 sequences and overlong encodings.
 * If an invalid sequence is detected, it returns ARCHI_STRING_UNICODE_CODEPOINT_REPLACEMENT
 * and the estimated length of the invalid sequence.
 * The decoder assumes the input is properly null-terminated and at least as long as the sequence.
 *
 * @return Unicode code point of the first character,
 * or ARCHI_STRING_UNICODE_CODEPOINT_REPLACEMENT if the string is invalid.
 */
uint32_t
archi_string_decode_utf8(
        const char *string,   ///< [in] UTF-8 string.
        size_t string_length, ///< [in] String length in bytes.
        size_t *char_length   ///< [out] Length of the first character in bytes.
);

/**
 * @brief Encode a Unicode code point and write the character to a string.
 *
 * String pointer is optional and may be set to NULL.
 * Non-null pointer must point to storage of enough size.
 *
 * @return Number of bytes written, or 0 if the code point is invalid.
 */
size_t
archi_string_encode_utf8(
        char *string, ///< [out] UTF-8 character string.
        uint32_t code_point ///< [in] Unicode code point.
);

#endif // _ARCHIPELAGO_UTIL_STRING_FUN_H_

