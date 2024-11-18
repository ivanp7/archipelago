/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Functions for printing formatted messages and logging.
 */

#pragma once
#ifndef _ARCHI_UTIL_PRINT_FUN_H_
#define _ARCHI_UTIL_PRINT_FUN_H_

/**
 * @brief Print arbitrary formatted text to standard error stream.
 *
 * Format string can be NULL, in which case nothing is printed.
 */
void
archi_print(
        const char *format, ///< [in] Output format.
        ... ///< [in] Values to format into output string.
);

/*****************************************************************************/

/**
 * @brief Get log verbosity level.
 *
 * @return Log verbosity level.
 */
int
archi_log_verbosity(void);

/**
 * @brief Set log verbosity level.
 *
 * This function has effect only the first time called,
 * which is usually done automatically.
 */
void
archi_log_set_verbosity(
        int verbosity_level ///< [in] Default log verbosity level.
);

/*****************************************************************************/

struct timespec;

/**
 * @brief Get log start time.
 */
void
archi_log_start_time(
        struct timespec *ts ///< [out] Log start time.
);

/**
 * @brief Set log start time.
 *
 * This function has effect only the first time called,
 * which is usually done automatically.
 */
void
archi_log_set_start_time(void);

/**
 * @brief Get log elapsed time.
 */
void
archi_log_elapsed_time(
        struct timespec *ts ///< [out] Log start time.
);

/*****************************************************************************/

/**
 * @brief Print formatted error line to standard error stream.
 *
 * If verbosity level is too low, nothing is printed.
 * Format string can be NULL, in which case nothing is printed.
 *
 * @see ARCHI_LOG_ERROR
 */
void
archi_log_error(
        const char *module,  ///< [in] Module name.
        const char *format,  ///< [in] Output format.
        ... ///< [in] Values to format into output string.
);

/**
 * @brief Print formatted warning line to standard error stream.
 *
 * If verbosity level is too low, nothing is printed.
 * Format string can be NULL, in which case nothing is printed.
 *
 * @see ARCHI_LOG_WARNING
 */
void
archi_log_warning(
        const char *module,  ///< [in] Module name.
        const char *format,  ///< [in] Output format.
        ... ///< [in] Values to format into output string.
);

/**
 * @brief Print formatted notice line to standard error stream.
 *
 * If verbosity level is too low, nothing is printed.
 * Format string can be NULL, in which case nothing is printed.
 *
 * @see ARCHI_LOG_NOTICE
 */
void
archi_log_notice(
        const char *module,  ///< [in] Module name.
        const char *format,  ///< [in] Output format.
        ... ///< [in] Values to format into output string.
);

/**
 * @brief Print formatted info line to standard error stream.
 *
 * If verbosity level is too low, nothing is printed.
 * Format string can be NULL, in which case nothing is printed.
 *
 * @see ARCHI_LOG_INFO
 */
void
archi_log_info(
        const char *module,  ///< [in] Module name.
        const char *format,  ///< [in] Output format.
        ... ///< [in] Values to format into output string.
);

/**
 * @brief Print formatted debug line to standard error stream.
 *
 * If verbosity level is too low, nothing is printed.
 * Format string can be NULL, in which case nothing is printed.
 *
 * @see ARCHI_LOG_DEBUG
 */
void
archi_log_debug(
        const char *module,  ///< [in] Module name.
        const char *format,  ///< [in] Output format.
        ... ///< [in] Values to format into output string.
);

#endif // _ARCHI_UTIL_PRINT_FUN_H_

