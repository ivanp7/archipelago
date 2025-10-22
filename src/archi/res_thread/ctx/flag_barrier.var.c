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
 * @brief Application context interface for flag barrier contexts.
 */

#include "archi/res_thread/ctx/flag_barrier.var.h"
#include "archi/res_thread/api/flag_barrier.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archi_context_thread_flag_barrier_init)
{
    if (params != NULL)
        return ARCHI_STATUS_EKEY;

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;

    archi_thread_flag_barrier_t barrier = archi_thread_flag_barrier_alloc(&code);
    if (barrier == NULL)
        return code;

    *context_data = (archi_pointer_t){
        .ptr = barrier,
        .element = {
            .num_of = 1,
        },
    };

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_thread_flag_barrier_final)
{
    archi_thread_flag_barrier_destroy(context->ptr);
}

const archi_context_interface_t archi_context_thread_flag_barrier_interface = {
    .init_fn = archi_context_thread_flag_barrier_init,
    .final_fn = archi_context_thread_flag_barrier_final,
};

