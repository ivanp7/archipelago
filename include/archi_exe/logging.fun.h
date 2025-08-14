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
 * @brief Operations on the global log context.
 */

#pragma once
#ifndef _ARCHI_EXE_LOGGING_FUN_H_
#define _ARCHI_EXE_LOGGING_FUN_H_

#include "archipelago/log/context.fun.h"

#include <stdio.h> // for FILE
#include <stdbool.h>

/**
 * @brief Retrieve the global log context.
 *
 * @return Pointer to the global log context.
 */
archi_log_context_t
archi_exe_log_context(void);

/*****************************************************************************/

/**
 * @brief Initialize the log stream.
 *
 * This function sets the log stream the very first time it's called.
 * Subsequent invocations have no effect.
 *
 * @param[in] stream
 *     Valid stream used to print messages to. If NULL, stderr is used instead.
 */
void
archi_exe_log_init_stream(
        FILE *stream
);

/**
 * @brief Initialize the log start time.
 *
 * This function sets the log start time the very first time it's called.
 * Subsequent invocations have no effect.
 *
 * @note The current UTC time is used as the log start time.
 */
void
archi_exe_log_init_start_time(void);

/**
 * @brief Initialize the log verbosity level (first call only).
 *
 * This function sets the log verbosity level the very first time it's called.
 * Subsequent invocations have no effect.
 *
 * The provided @p level is clamped into the valid
 * range [0, ARCHI_LOG_VERBOSITY_MAX] before being stored.
 *
 * @param[in] level
 *     Desired verbosity level. Values < 0 become 0; values >
 *     ARCHI_LOG_VERBOSITY_MAX become ARCHI_LOG_VERBOSITY_MAX.
 */
void
archi_exe_log_init_verbosity(
        int level
);

/**
 * @brief Initialize the log color usage flag (first call only).
 *
 * This function sets the usage of color in log the very first time it's called.
 * Subsequent invocations have no effect.
 *
 * @param[in] colorful
 *     True to allow use of color in log messages, false to force monochrome output.
 */
void
archi_exe_log_init_color(
        bool colorful
);

#endif // _ARCHI_EXE_LOGGING_FUN_H_

