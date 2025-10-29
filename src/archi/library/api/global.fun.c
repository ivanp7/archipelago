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
 * @brief Functions for initialization of library global object pointers.
 */

#include "archi/library/api/global.fun.h"
#include "archi/library/api/handle.fun.h"

#include <stddef.h> // for NULL

bool
archi_library_initialize_global(
        archi_library_handle_t handle,
        archi_global_init_spec_t init_spec)
{
    if (handle == NULL)
        return false;
    else if ((init_spec.get_fn == NULL) || (init_spec.set_fn_symbol == NULL))
        return false;

    union { // type punning valid on POSIX systems
        archi_global_set_func_t context_setter_fn;
        void *symbol;
    } library = {.symbol = archi_library_get_symbol(handle, init_spec.set_fn_symbol)};

    if (library.symbol == NULL)
        return false;

    void *context = init_spec.get_fn();
    library.context_setter_fn(context);

    return true;
}

