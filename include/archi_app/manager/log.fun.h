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
 * @brief The logging manager operations.
 */

#pragma once
#ifndef _ARCHI_APP_MANAGER_LOG_FUN_H_
#define _ARCHI_APP_MANAGER_LOG_FUN_H_

#include "archi_base/manager.typ.h"

#include <stdio.h> // for FILE
#include <stdbool.h>


/**
 * @brief Retrieve the logging manager.
 *
 * @return Pointer to the logging manager.
 */
ARCHI_MANAGER_PTR_GET_FUNC(archi_app_manager_ptr_get__log);

/*****************************************************************************/

/**
 * @brief Initialize the logging manager.
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
 * @param[in] indentation
 *     Desired intentation width. Values < 0 become 0.
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
archi_app_manager_initialize__log(
        FILE *stream,
        int level,
        int indentation,
        bool colorful
);

/**
 * @brief Finalize the logging manager.
 */
void
archi_app_manager_finalize__log(void);

#endif // _ARCHI_APP_MANAGER_LOG_FUN_H_

