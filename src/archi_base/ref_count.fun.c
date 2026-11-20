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
archi_refcount_total_allocations(void)
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
archi_refcount_total_releases(void)
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
archi_refcount_total_increments(void)
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
archi_refcount_total_decrements(void)
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
archi_refcount_error_detected(void)
{
    if (archi_ref_count_manager == NULL)
        return false;

#ifndef __STDC_NO_ATOMICS__
    return atomic_load_explicit(&archi_ref_count_manager->error_detected,
            memory_order_relaxed);
#else
    return archi_ref_count_manager->error_detected;
#endif
}

static
uintmax_t
archi_refcount_total_add_allocation(void)
{
    if (archi_ref_count_manager == NULL)
        return -1;

#ifndef __STDC_NO_ATOMICS__
    return atomic_fetch_add_explicit(&archi_ref_count_manager->num_allocations, 1,
            memory_order_release);
#else
    return archi_ref_count_manager->num_allocations++;
#endif
}

static
void
archi_refcount_total_add_release(void)
{
    if (archi_ref_count_manager == NULL)
        return;

#ifndef __STDC_NO_ATOMICS__
    atomic_fetch_add_explicit(&archi_ref_count_manager->num_releases, 1,
            memory_order_release);
#else
    archi_ref_count_manager->num_releases++;
#endif
}

static
void
archi_refcount_total_add_increment(void)
{
    if (archi_ref_count_manager == NULL)
        return;

#ifndef __STDC_NO_ATOMICS__
    atomic_fetch_add_explicit(&archi_ref_count_manager->num_increments, 1,
            memory_order_release);
#else
    archi_ref_count_manager->num_increments++;
#endif
}

static
void
archi_refcount_total_add_decrements(
        uintmax_t count)
{
    if (archi_ref_count_manager == NULL)
        return;

#ifndef __STDC_NO_ATOMICS__
    atomic_fetch_add_explicit(&archi_ref_count_manager->num_decrements, count,
            memory_order_release);
#else
    archi_ref_count_manager->num_decrements += count;
#endif
}

static
void
archi_refcount_set_error_flag(void)
{
    if (archi_ref_count_manager == NULL)
        return;

#ifndef __STDC_NO_ATOMICS__
    atomic_store_explicit(&archi_ref_count_manager->error_detected, true,
            memory_order_relaxed);
#else
    archi_ref_count_manager->error_detected = true;
#endif
}

/*****************************************************************************/

struct archi_refcount {
    uintmax_t id; // globally unique ID for debugging purposes

#ifndef __STDC_NO_ATOMICS__
    atomic_uintmax_t value; // reference count
#else
    uintmax_t value; // reference count
#endif

    archi_destructor_func_t destructor_fn; // object destructor
    void *destructor_data; // object destructor data
};

archi_refcount_t
archi_refcount_alloc(
        archi_destructor_func_t destructor_fn,
        void *destructor_data)
{
    archi_refcount_t ref_count = malloc(sizeof(*ref_count));
    if (ref_count == NULL)
        return NULL;

    // Set the counter ID and increment the global counter of allocations
    ref_count->id = 1 + archi_refcount_total_add_allocation();

    // Increment the global counter of increments
    archi_refcount_total_add_increment();

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
archi_refcount_free(
        archi_refcount_t ref_count)
{
    if (ref_count == NULL)
        return;

    // Increment the global counter of releases
    archi_refcount_total_add_release();

    // Fetch the current reference count
    uintmax_t value =
#ifndef __STDC_NO_ATOMICS__
        atomic_load_explicit(&ref_count->value, memory_order_acquire);
#else
        ref_count->value;
#endif

    if (value == 0) // should not happen
    {
        archi_refcount_set_error_flag();
        return;
    }

    // Add the current count to the global counter of decrements
    // to avoid false positives by memory leak detection
    archi_refcount_total_add_decrements(value);

    // Release the reference counter
    ref_count->destructor_fn = NULL; // unnecessary, but improves robustness
    free(ref_count);
}

void
archi_refcount_increment(
        archi_refcount_t ref_count)
{
    if (ref_count == NULL)
        return;

    // Increment the global counter of increments
    archi_refcount_total_add_increment();

    // Increment the reference counter
    uintmax_t old_value =
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_add_explicit(&ref_count->value, 1, memory_order_release);
#else
        ref_count->value++;
#endif

    if (old_value == 0) // should not happen
        archi_refcount_set_error_flag();
}

bool
archi_refcount_decrement(
        archi_refcount_t ref_count)
{
    if (ref_count == NULL)
        return false;

    // Increment the global counter of decrements
    archi_refcount_total_add_decrements(1);

    // Decrement the reference counter
    uintmax_t old_value =
#ifndef __STDC_NO_ATOMICS__
        atomic_fetch_sub_explicit(&ref_count->value, 1, memory_order_release);
#else
        ref_count->value--;
#endif

    if (old_value == 0) // should not happen
    {
        archi_refcount_set_error_flag();
        return false;
    }

    // Check if the counter has reached zero
    if (old_value > 1)
        return false; // it hasn't

    // Increment the global counter of releases
    archi_refcount_total_add_release();

    // Synchronize memory before destroying the object
#ifndef __STDC_NO_ATOMICS__
    atomic_thread_fence(memory_order_acquire);
#endif

    // Call the destructor
    if (ref_count->destructor_fn != NULL)
        /***************************************************/
        ref_count->destructor_fn(ref_count->destructor_data);
        /***************************************************/

    // Release the reference counter
    ref_count->destructor_fn = NULL; // unnecessary, but improves robustness
    free(ref_count);

    return true;
}

uintmax_t
archi_refcount_id(
        archi_refcount_t ref_count)
{
    if (ref_count == NULL)
        return 0;

    return ref_count->id;
}

