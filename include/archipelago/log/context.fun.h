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
 * @brief Operations on the global logging context.
 */

#pragma once
#ifndef _ARCHIPELAGO_LOG_CONTEXT_FUN_H_
#define _ARCHIPELAGO_LOG_CONTEXT_FUN_H_

#include "archipelago/base/global.typ.h"

#include <stdbool.h>

struct timespec;

/**
 * @brief Initialize the logging context pointer.
 *
 * @pre
 *      context != NULL
 *
 * @post
 *      Logging API is ready to be used.
 *
 * @note Subsequent invocations have no effect.
 *
 * @param[in] context
 *      Logging context.
 */
ARCHI_GLOBAL_SET_FUNC(archi_log_global_context_set);

/**
 * @brief Retrieve the logging context pointer.
 *
 * If the logging context pointer hasn't been initialized yet, the function returns NULL.
 *
 * @return Pointer to the log context.
 */
ARCHI_GLOBAL_GET_FUNC(archi_log_global_context);

/*****************************************************************************/

/**
 * @brief Calculate and return the elapsed time since logging started.
 *
 * This function computes the difference between the current UTC time
 * and the log start time, returning it as a struct timespec in UTC.
 *
 * @param[out] ts
 *     Pointer to a struct timespec where the elapsed time will be stored.
 *     Must not be NULL.
 */
void
archi_log_elapsed_time(
        struct timespec *ts
);

/**
 * @brief Retrieve the log verbosity level.
 *
 * If the logger is not initialized, it returns ARCHI_LOG_VERBOSITY_QUIET,
 * which typically suppresses most log output.
 *
 * @return
 *   Log verbosity level. Higher values generally correspond to more verbose logging.
 */
int
archi_log_verbosity(void);

/**
 * @brief Determine whether colored output is enabled for log messages.
 *
 * Checks the logger context to see if log messages
 * should include ANSI color codes to produce
 * colored output on supported terminals.
 * If the logger is not initialized, this function returns false.
 *
 * @return
 *   true if colored output is enabled for logs; false otherwise.
 */
bool
archi_log_colorful(void);

#endif // _ARCHIPELAGO_LOG_CONTEXT_FUN_H_

