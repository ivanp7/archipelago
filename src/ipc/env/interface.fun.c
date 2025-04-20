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
 * @brief Environmental variable operations.
 */

#include "archi/ipc/env/interface.fun.h"

#include <stdlib.h> // for getenv(), malloc()
#include <string.h> // for strlen(), memcpy()
#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h>
#endif

char*
archi_env_get(
        const char *name,
        archi_status_t *code)
{
#ifndef __STDC_NO_ATOMICS__
    static atomic_flag spinlock = ATOMIC_FLAG_INIT;
#endif

    if (name == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    char *value = NULL;

#ifndef __STDC_NO_ATOMICS__
    while (atomic_flag_test_and_set_explicit(&spinlock, memory_order_acquire)); // lock
#endif

    const char *value_orig = getenv(name);
    if (value_orig == NULL)
    {
        if (code != NULL)
            *code = 1; // env. variable not found

        goto finish;
    }

    size_t valuesz = strlen(value_orig) + 1;

    value = malloc(valuesz);
    if (value == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto finish;
    }

    memcpy(value, value_orig, valuesz);

    if (code != NULL)
        *code = 0;

finish:
#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear_explicit(&spinlock, memory_order_release); // unlock
#endif

    return value;
}

