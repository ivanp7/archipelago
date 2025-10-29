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
 * @brief Operations on the global logging context.
 */

#pragma once
#ifndef _ARCHI_EXE_LOGGING_FUN_H_
#define _ARCHI_EXE_LOGGING_FUN_H_

#include "archipelago/base/global.typ.h"

#include <stdio.h> // for FILE
#include <stdbool.h>

/**
 * @brief Retrieve the global log context.
 *
 * @return Pointer to the global log context.
 */
ARCHI_GLOBAL_GET_FUNC(archi_exe_log_global_context);

/*****************************************************************************/

/**
 * @brief Initialize the global log context.
 *
 * This function sets the current UTC time as the log start time.
 *
 * @param[in] stream
 *     Valid stream used to print messages to. If NULL, the default stream (stderr) is used instead.
 *
 * @param[in] level
 *     Desired verbosity level. Values < 0 become 0; values >
 *     ARCHI_LOG_VERBOSITY_MAX become ARCHI_LOG_VERBOSITY_MAX.
 *
 * @param[in] colorful
 *     True to allow use of color in log messages, false to force monochrome output.
 *
 * The provided @p level is clamped into the valid
 * range [0, ARCHI_LOG_VERBOSITY_MAX] before being stored.
 *
 * @return True on success, false on failure.
 */
bool
archi_exe_log_initialize(
        FILE *stream,
        int level,
        bool colorful
);

/**
 * @brief Finalize the global log context.
 */
void
archi_exe_log_finalize(void);

#endif // _ARCHI_EXE_LOGGING_FUN_H_

