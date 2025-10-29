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
#ifndef _ARCHIPELAGO_LOG_PRINT_FUN_H_
#define _ARCHIPELAGO_LOG_PRINT_FUN_H_

#include <stdbool.h>

/**
 * @brief Print formatted text to the log stream using a printf-style format string.
 *
 * If @p format is NULL, the function returns immediately without printing.
 *
 * @param[in] format
 *     A null-terminated printf-style format string. May be NULL,
 *     in which case no output is generated.
 *
 * @param[in] ...
 *     Zero or more arguments matching the format specifiers in @p format.
 */
void
archi_print(
        const char *format,
        ...
);

/**
 * @brief Print string to the log stream only if ANSI escape codes are enabled.
 *
 * If @p color is NULL, the function does nothing.
 *
 * @param[in] string
 *     A null-terminated string. May be NULL, in which case no output is generated.
 */
void
archi_print_escape_code(
        const char *string
);

#define archi_print_color archi_print_escape_code

/**
 * @brief Lock an internal mutex to protect a composite printing operation.
 *
 * Internally, this routine uses a mutex to serialize concurrent calls
 * and prevent interleaved output from multiple threads.
 *
 * @note The user code must not use archi_print() and archi_print_color()
 * outside of a lock-protected section.
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

#endif // _ARCHIPELAGO_LOG_PRINT_FUN_H_

