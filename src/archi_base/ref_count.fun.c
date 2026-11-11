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
 * @brief Reference counting operations.
 */

#include "archi_base/ref_count.fun.h"
#include "archi_base/ref_count_manager.fun.h"
#include "ref_count_manager.typ.h"

#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h> // for atomic_size_t, atomic_*
#else
#  include <stddef.h> // for size_t
#endif
#include <stdlib.h> // for malloc(), free()


static
struct archi_ref_count_manager *archi_ref_count_manager;

ARCHI_MANAGER_PTR_GET_FUNC(archi_manager_ptr_get__ref_count)
{
    return archi_ref_count_manager;
}

ARCHI_MANAGER_PTR_INIT_FUNC(archi_manager_ptr_init__ref_count)
{
    if (archi_ref_count_manager != NULL)
        return;

    archi_ref_count_manager = manager;
}

/*****************************************************************************/

unsigned long long
archi_reference_count_total_increments(void)
{
    if (archi_ref_count_manager == NULL)
        return 0;

#ifndef __STDC_NO_ATOMICS__
    return atomic_load_explicit(&archi_ref_count_manager->num_increments,
            memory_order_acquire);
#else
    return archi_ref_count_manager->num_increments;
#endif
}

unsigned long long
archi_reference_count_total_decrements(void)
{
    if (archi_ref_count_manager == NULL)
        return 0;

#ifndef __STDC_NO_ATOMICS__
    return atomic_load_explicit(&archi_ref_count_manager->num_decrements,
            memory_order_acquire);
#else
    return archi_ref_count_manager->num_decrements;
#endif
}

bool
archi_reference_count_negative_detected(void)
{
    if (archi_ref_count_manager == NULL)
        return false;

#ifndef __STDC_NO_ATOMICS__
    return atomic_load_explicit(&archi_ref_count_manager->negative_count,
            memory_order_relaxed);
#else
    return archi_ref_count_manager->negative_count;
#endif
}

/*****************************************************************************/

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

    if (archi_ref_count_manager != NULL)
    {
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&archi_ref_count_manager->num_increments, 1,
                memory_order_release);
#else
        archi_ref_count_manager->num_increments++;
#endif
    }

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

    if (archi_ref_count_manager != NULL)
    {
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&archi_ref_count_manager->num_increments, 1,
                memory_order_release);
#else
        archi_ref_count_manager->num_increments++;
#endif
    }

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

    if (archi_ref_count_manager != NULL)
    {
        unsigned long long num_decrements = 1 + // calculating the new value from the old
#ifndef __STDC_NO_ATOMICS__
            atomic_fetch_add_explicit(&archi_ref_count_manager->num_decrements, 1,
                    memory_order_release);
#else
            archi_ref_count_manager->num_decrements++;
#endif

        unsigned long long num_increments = archi_reference_count_total_increments();

        if (num_increments < num_decrements) // memory error detected
        {
#ifndef __STDC_NO_ATOMICS__
            atomic_store_explicit(&archi_ref_count_manager->negative_count,
                    true, memory_order_relaxed);
#else
            archi_ref_count_manager->negative_count = true;
#endif
        }
    }

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

