/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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

/**
 * @brief Thread-safe, printf-style output to the standard error stream.
 *
 * Prints formatted text to the log stream using a printf-style format string.
 * If @p format is NULL, the function returns immediately without printing.
 *
 * Internally, this routine uses an atomic spinlock (when C11 atomics
 * are available) to serialize concurrent calls and prevent interleaved
 * output from multiple threads.
 *
 * @param[in] format
 *     A null-terminated printf-style format string. May be NULL,
 *     in which case no output is generated.
 * @param[in] ...
 *     Zero or more arguments matching the format specifiers in @p format.
 *
 * @note Thread safe when compiled with C11 atomics; otherwise,
 *       thread safety is not guaranteed (output may interleave in multithreaded scenarios).
 */
void
archi_print(
        const char *format,
        ...
);

/*****************************************************************************/

/**
 * @brief Log an error-level message.
 *
 * Each message is colorized, prefixed with an elapsed-time timestamp,
 * a single-character level indicator, and optionally the module name.
 * Messages are emitted only if the current verbosity allows it.
 * Thread-safe via an atomic spinlock around the actual print.
 *
 * @param[in] module
 *     Optional null-terminated module name;
 *     if non-NULL it’s printed before the user message.
 *
 * @param[in] format
 *     printf-style format string.  If NULL, no user-text is printed
 *     (only timestamp/module prefix appears).
 *
 * @param[in] ...
 *     Arguments corresponding to @p format.
 */
void
archi_log_error(
        const char *module,
        const char *format,
        ...
);

/**
 * @brief Log a warning-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the warning color/indicator.
 *
 * @param[in] module
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
        const char *module,
        const char *format,
        ...
);

/**
 * @brief Log a notice-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the notice color/indicator.
 *
 * @param[in] module
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
        const char *module,
        const char *format,
        ...
);

/**
 * @brief Log a info-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the info color/indicator.
 *
 * @param[in] module
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
        const char *module,
        const char *format,
        ...
);

/**
 * @brief Log a debug-level message.
 *
 * Same behavior as archi_log_error(), but gated at
 * higher verbosity level and using the debug color/indicator.
 *
 * @param[in] module
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
        const char *module,
        const char *format,
        ...
);

#endif // _ARCHI_LOG_PRINT_FUN_H_

