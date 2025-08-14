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
 * @brief Functions for library logging initialization.
 */

#pragma once
#ifndef _ARCHI_RES_LIBRARY_API_LOGGING_FUN_H_
#define _ARCHI_RES_LIBRARY_API_LOGGING_FUN_H_

#include <stdbool.h>

/**
 * @brief Synchronize a loaded library's logging module with the host application.
 *
 * When a shared library that uses the Archi logging API is loaded at runtime,
 * its internal logging module must be initialized with a log context.
 * This function attempts to locate the exported symbol in the library:
 *   - archi_log_initialize()
 *
 * If found, it is invoked and provided with the result of archi_log_get_context() call.
 * If the symbol is not present, the function does nothing.
 *
 * @pre
 *   - The logging system has been initialized in the host application.
 *   - @p handle was successfully opened and refers to a valid shared library handle.
 *
 * @post
 *   - If the symbol "archi_log_initialize" exists in the library, it is
 *     called with the current log context.
 *
 * @param[in] handle
 *   The dynamic library handle (e.g. as returned by archi_library_load()).
 *   If NULL, no action is taken.
 *
 * @return True on success, otherwise false.
 *
 * @par Example
 * @code
 *   // Load a plugin and sync its logging to match the application
 *   void *plugin = archi_library_load(params);
 *   if (plugin == NULL) {
 *       archi_log_error("Failed to load plugin");
 *       return;
 *   }
 *   archi_log_initialize_library(plugin);
 *
 *   // Now the plugin logging module is ready to be used.
 * @endcode
 */
bool
archi_library_initialize_logging(
        void *handle
);

#endif // _ARCHI_RES_LIBRARY_API_LOGGING_FUN_H_

