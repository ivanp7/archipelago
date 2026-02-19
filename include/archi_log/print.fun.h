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
 * @brief Functions for logging and printing formatted messages.
 */

#pragma once
#ifndef _ARCHI_LOG_PRINT_FUN_H_
#define _ARCHI_LOG_PRINT_FUN_H_

#include <stdbool.h>
#include <stddef.h> // for size_t


/**
 * @brief Lock an internal mutex to protect a composite printing operation.
 *
 * Internally, this routine uses a mutex to serialize concurrent calls
 * and prevent interleaved output from multiple threads.
 *
 * @warning Each archi_print_lock() must be paired by the corresponding archi_print_unlock().
 *
 * @param[in] verbosity
 *     Minimum verbosity level at which the mutex is acquired.
 *
 * @return True if the mutex is acquired, false otherwise.
 */
bool
archi_print_lock(
        int verbosity
);

/**
 * @brief Unock an internal mutex to protect a composite printing operation.
 *
 * @warning Each archi_print_unlock() must be paired by the corresponding archi_print_lock().
 */
void
archi_print_unlock(void);

/*****************************************************************************/

/**
 * @brief Print formatted text to the log stream using a printf-style format string.
 *
 * @note This function must be protected by `archi_print_lock()`.
 *
 * If @p format is NULL, the function returns immediately without printing.
 *
 * @param[in] format
 *     Null-terminated printf-style format string.
 *
 * @param[in] ...
 *     Zero or more arguments matching the format specifiers in @p format.
 *
 * @return Number of printed characters, or negative value on error.
 */
int
archi_print(
        const char *format,
        ...
);

/**
 * @brief Print string to the log stream.
 *
 * @note This function must be protected by `archi_print_lock()`.
 *
 * If @p color is NULL, the function does nothing.
 *
 * @param[in] string
 *     Null-terminated string.
 *
 * @return Number of printed characters, or negative value on error.
 */
int
archi_print_string(
        const char *string
);

/**
 * @brief Print string to the log stream (only if color is enabled).
 *
 * @note This function must be protected by `archi_print_lock()`.
 *
 * If @p color is NULL, the function does nothing.
 *
 * @param[in] string
 *     Null-terminated string.
 *
 * @return Number of printed characters, or negative value on error.
 */
int
archi_print_color(
        const char *string
);

/**
 * @brief Print hex dump of memory to the log stream.
 *
 * @note This function must be protected by `archi_print_lock()`.
 *
 * The dumped memory range is [memory; memory + length).
 * If @p memory is NULL or @p length is 0, the function does nothing.
 *
 * @param[in] memory
 *     Pointer to memory.
 *
 * @param[in] length
 *     Length of memory in bytes.
 *
 * @param[in] absolute
 *     If true, print memory addresses in the left column, otherwise print offsets relative the the memory pointer.
 *
 * @param[in] indentation
 *     Number of indentation spaces at the beginning of each output line.
 */
void
archi_print_hex_dump(
        const void *memory,
        size_t length,
        bool absolute,
        unsigned indentation
);

/*****************************************************************************/

/**
 * @brief Log an error-level message.
 *
 * Each message is colorized, prefixed with an elapsed-time timestamp,
 * a single-character level indicator, and optionally the message origin name.
 * Messages are emitted only if the current verbosity allows it.
 *
 * Thread-safe with the help of a mutex protecting the actual print,
 * (as if implicitly surrounded by a pair of archi_print_lock() and archi_print_unlock()).
 *
 * @param[in] origin
 *     Optional null-terminated message origin name;
 *     if non-NULL it’s printed before the user message.
 *
 * @param[in] format
 *     printf-style format string.  If NULL, no user-text is printed
 *     (only timestamp/origin prefix appears).
 *
 * @param[in] ...
 *     Arguments corresponding to @p format.
 */
void
archi_log_error(
        const char *origin,
        const char *format,
        ...
);

/**
 * @brief Log a warning-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the warning color/indicator.
 *
 * @param[in] origin
 *     Same as in archi_log_error().
 *
 * @param[in] format
 *     Same as in archi_log_error().
 *
 * @param[in] ...
 *     Same as in archi_log_error().
 */
void
archi_log_warning(
        const char *origin,
        const char *format,
        ...
);

/**
 * @brief Log a notice-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the notice color/indicator.
 *
 * @param[in] origin
 *     Same as in archi_log_error().
 *
 * @param[in] format
 *     Same as in archi_log_error().
 *
 * @param[in] ...
 *     Same as in archi_log_error().
 */
void
archi_log_notice(
        const char *origin,
        const char *format,
        ...
);

/**
 * @brief Log a info-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the info color/indicator.
 *
 * @param[in] origin
 *     Same as in archi_log_error().
 *
 * @param[in] format
 *     Same as in archi_log_error().
 *
 * @param[in] ...
 *     Same as in archi_log_error().
 */
void
archi_log_info(
        const char *origin,
        const char *format,
        ...
);

/**
 * @brief Log a debug-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the debug color/indicator.
 *
 * @param[in] origin
 *     Same as in archi_log_error().
 *
 * @param[in] format
 *     Same as in archi_log_error().
 *
 * @param[in] ...
 *     Same as in archi_log_error().
 */

void
archi_log_debug(
        const char *origin,
        const char *format,
        ...
);

#endif // _ARCHI_LOG_PRINT_FUN_H_

