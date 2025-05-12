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
 * @brief Reference counter interface.
 */

#include "archi/util/ref_count.fun.h"

#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h> // for atomic_size_t, atomic_*
#else
#  include <stddef.h> // for size_t
#endif
#include <stdlib.h> // for malloc(), free()

struct archi_reference_count {
#ifndef __STDC_NO_ATOMICS__
    atomic_size_t
#else
    size_t
#endif
        value;

    archi_destructor_func_t destructor_fn;
    void *destructor_data;
};

archi_reference_count_t
archi_reference_count_alloc(
        archi_destructor_func_t destructor_fn,
        void *destructor_data)
{
    if (destructor_fn == NULL)
        return NULL;

    archi_reference_count_t ref_count = malloc(sizeof(*ref_count));
    if (ref_count == NULL)
        return NULL;

#ifndef __STDC_NO_ATOMICS__
    atomic_init(&ref_count->value, 1);
#else
    ref_count->value = 1;
#endif

    ref_count->destructor_fn = destructor_fn;
    ref_count->destructor_data = destructor_data;

    return ref_count;
}

void
archi_reference_count_free(
        archi_reference_count_t ref_count)
{
    free(ref_count);
}

void
archi_reference_count_increment(
        archi_reference_count_t ref_count)
{
    if (ref_count == NULL)
        return;

#ifndef __STDC_NO_ATOMICS__
    atomic_fetch_add_explicit(&ref_count->value, 1, memory_order_relaxed);
#else
    ref_count->value++;
#endif
}

bool
archi_reference_count_decrement(
        archi_reference_count_t ref_count)
{
    if (ref_count == NULL)
        return false;

#ifndef __STDC_NO_ATOMICS__
    if (atomic_fetch_sub_explicit(&ref_count->value, 1, memory_order_release) == 1)
    {
        atomic_thread_fence(memory_order_acquire);
        goto destroy;
    }
#else
    if (--ref_count->value == 0)
        goto destroy;
#endif

    return false;

destroy:
    ref_count->destructor_fn(ref_count->destructor_data);
    archi_reference_count_free(ref_count);

    return true;
}

