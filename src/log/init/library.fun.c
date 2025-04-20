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

#include "archi/log/init/library.fun.h"
#include "archi/log/context.fun.h"
#include "archi/res/library/interface.fun.h" // for archi_library_get_symbol()

#include <stddef.h> // for NULL

bool
archi_log_initialize_library(
        void *handle)
{
    if (handle == NULL)
        return false;

    struct archi_log_context *context = archi_log_get_context();
    if (context == NULL)
        return false;

    void *symbol = archi_library_get_symbol(handle, "archi_log_initialize");
    if (symbol == NULL)
        return false;

    archi_log_initialize_func_t *fptr =
        (archi_log_initialize_func_t*)&symbol; // valid cast on POSIX systems

    (*fptr)(context); // equivalent to archi_log_initialize(context);
    return true;
}

