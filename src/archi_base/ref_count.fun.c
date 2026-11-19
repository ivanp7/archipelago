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
#  include <stdatomic.h> // for atomic_*
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

uintmax_t
archi_reference_count_total_allocations(void)
{
    if (archi_ref_count_manager == NULL)
        return 0;

#ifndef __STDC_NO_ATOMICS__
    return atomic_load_explicit(&archi_ref_count_manager->num_allocations,
            memory_order_acquire);
#else
    return archi_ref_count_manager->num_allocations;
#endif
}

uintmax_t
archi_reference_count_total_releases(void)
{
    if (archi_ref_count_manager == NULL)
        return 0;

#ifndef __STDC_NO_ATOMICS__
    return atomic_load_explicit(&archi_ref_count_manager->num_releases,
            memory_order_acquire);
#else
    return archi_ref_count_manager->num_releases;
#endif
}

uintmax_t
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

uintmax_t
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
    uintmax_t id; // globally unique ID for debugging purposes

#ifndef __STDC_NO_ATOMICS__
    atomic_uintmax_t value; // reference count
#else
    uintmax_t value; // reference count
#endif

    archi_destructor_func_t destructor_fn; // object destructor
    void *destructor_data; // object destructor data
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
        // Set the counter ID and increment the global counter of allocations
        ref_count->id = 1 +
#ifndef __STDC_NO_ATOMICS__
            atomic_fetch_add_explicit(&archi_ref_count_manager->num_allocations, 1,
                    memory_order_release);
#else
            archi_ref_count_manager->num_allocations++;
#endif

        // Increment the global counter of increments
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&archi_ref_count_manager->num_increments, 1,
                memory_order_release);
#else
        archi_ref_count_manager->num_increments++;
#endif
    }
    else
        ref_count->id = 0; // ID is not set

    // Initialize the reference counter
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
    if (ref_count == NULL)
        return;

    if (archi_ref_count_manager != NULL)
    {
        // Increment the global counter of releases
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&archi_ref_count_manager->num_releases, 1,
                memory_order_release);
#else
        archi_ref_count_manager->num_releases++;
#endif

        // Fetch the current reference count
        uintmax_t count =
#ifndef __STDC_NO_ATOMICS__
            atomic_load_explicit(&ref_count->value, memory_order_acquire);
#else
            ref_count->value;
#endif

        // Add the current count to the global counter of decrements (to avoid false positives for memory leak detection)
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&archi_ref_count_manager->num_decrements, count,
                memory_order_release);
#else
        archi_ref_count_manager->num_decrements += count;
#endif
    }

    // Release the reference counter
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
        // Increment the global counter of increments
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&archi_ref_count_manager->num_increments, 1,
                memory_order_release);
#else
        archi_ref_count_manager->num_increments++;
#endif
    }

    // Increment the reference counter
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
        // Fetch and increment the global counter of decrements
        uintmax_t num_decrements = 1 + // calculating the new value from the old
#ifndef __STDC_NO_ATOMICS__
            atomic_fetch_add_explicit(&archi_ref_count_manager->num_decrements, 1,
                    memory_order_release);
#else
            archi_ref_count_manager->num_decrements++;
#endif

        // Fetch the global counter of increments
        uintmax_t num_increments = archi_reference_count_total_increments();

        // Check for the negative count error
        if (num_increments < num_decrements)
        {
#ifndef __STDC_NO_ATOMICS__
            atomic_store_explicit(&archi_ref_count_manager->negative_count,
                    true, memory_order_relaxed);
#else
            archi_ref_count_manager->negative_count = true;
#endif
        }
    }

    // Decrement the reference counter
#ifndef __STDC_NO_ATOMICS__
    if (atomic_fetch_sub_explicit(&ref_count->value, 1, memory_order_release) == 1)
    {
        atomic_thread_fence(memory_order_acquire); // synchronize memory before destroying the object
        goto destroy;
    }
#else
    if (--ref_count->value == 0)
        goto destroy;
#endif

    return false;

destroy:
    // Call the destructor
    ref_count->destructor_fn(ref_count->destructor_data);

    if (archi_ref_count_manager != NULL)
    {
        // Increment the global counter of releases
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&archi_ref_count_manager->num_releases, 1,
                memory_order_release);
#else
        archi_ref_count_manager->num_releases++;
#endif
    }

    // Release the reference counter
    free(ref_count);

    return true;
}

uintmax_t
archi_reference_count_id(
        archi_reference_count_t ref_count)
{
    if (ref_count == NULL)
        return 0;

    return ref_count->id;
}

