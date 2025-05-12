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
 * @brief Lock-free queue operations.
 */

#include "archi/ds/lfqueue/api.fun.h"
#include "archi/util/size.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy(), memset()

#include <stdatomic.h> // for atomic_uint_fast*_t
#include <stdint.h> // for uint_fast*_t
#include <limits.h> // for CHAR_BIT

#ifdef ARCHI_FEATURE_LFQUEUE32

typedef uint_fast32_t archi_lfqueue_count_t;
typedef uint_fast64_t archi_lfqueue_count2_t;

typedef atomic_uint_fast32_t archi_lfqueue_atomic_count_t;
typedef atomic_uint_fast64_t archi_lfqueue_atomic_count2_t;

#else

typedef uint_fast16_t archi_lfqueue_count_t;
typedef uint_fast32_t archi_lfqueue_count2_t;

typedef atomic_uint_fast16_t archi_lfqueue_atomic_count_t;
typedef atomic_uint_fast32_t archi_lfqueue_atomic_count2_t;

#endif

struct archi_lfqueue {
    void *buffer;

    struct {
        size_t actual;
        size_t padded;
    } element_size;

    archi_lfqueue_count_t mask_bits;

    archi_lfqueue_atomic_count_t *push_count, *pop_count;
    archi_lfqueue_atomic_count2_t total_push_count, total_pop_count;
};

archi_lfqueue_t
archi_lfqueue_alloc(
        archi_lfqueue_alloc_params_t params,
        archi_status_t *code)
{
    if (params.capacity_log2 > sizeof(archi_lfqueue_count_t) * CHAR_BIT)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    // Calculate data buffer size
    archi_array_layout_t layout = {
        .num_of = (size_t)1 << params.capacity_log2,
        .size = params.element_size,
        .alignment = params.element_alignment,
    };

    size_t buffer_size = 0;
    if (params.element_size != 0)
    {
        if ((params.element_alignment == 0) ||
                ((params.element_alignment & (params.element_alignment - 1)) != 0))
        {
            if (code != NULL)
                *code = ARCHI_STATUS_EMISUSE;

            return NULL;
        }

        buffer_size = archi_size_array(layout);
        if (buffer_size == 0)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_EMISUSE;

            return NULL;
        }
    }

    // Allocate the queue object
    archi_lfqueue_t queue = malloc(sizeof(*queue));
    if (queue == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    // Allocate the data buffer
    void *buffer = NULL;
    if (buffer_size != 0)
    {
        buffer = aligned_alloc(params.element_alignment, buffer_size);
        if (buffer == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            free(queue);
            return NULL;
        }
    }

    // Initialize the queue object
    *queue = (struct archi_lfqueue){
        .buffer = buffer,
        .element_size = {
            .actual = params.element_size,
            .padded = (params.element_size == 0) ? 0 :
                ARCHI_SIZE_PADDED(params.element_size, params.element_alignment),
        },
        .mask_bits = params.capacity_log2,
    };

    // Allocate the arrays of counters
    buffer_size = sizeof(*queue->push_count) * layout.num_of;

    if (buffer_size / sizeof(*queue->push_count) != layout.num_of)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        free(queue->buffer);
        free(queue);
        return NULL;
    }

    queue->push_count = malloc(buffer_size);
    if (queue->push_count == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        free(queue->buffer);
        free(queue);
        return NULL;
    }

    queue->pop_count = malloc(buffer_size);
    if (queue->pop_count == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        free(queue->push_count);
        free(queue->buffer);
        free(queue);
        return NULL;
    }

    // Initialize all the counters
    for (size_t i = 0; i < layout.num_of; i++)
    {
        atomic_init(&queue->push_count[i], 0);
        atomic_init(&queue->pop_count[i], 0);
    }

    atomic_init(&queue->total_push_count, 0);
    atomic_init(&queue->total_pop_count, 0);

    if (code != NULL)
        *code = 0;

    return queue;
}

void
archi_lfqueue_free(
        archi_lfqueue_t queue)
{
    if (queue == NULL)
        return;

    free(queue->push_count);
    free(queue->pop_count);
    free(queue->buffer);
    free(queue);
}

bool
archi_lfqueue_push(
        archi_lfqueue_t queue,
        const void *value)
{
    if (queue == NULL)
        return false;

    archi_lfqueue_count_t mask_bits = queue->mask_bits;
    archi_lfqueue_count_t mask = ((archi_lfqueue_count_t)1 << mask_bits) - 1;

    archi_lfqueue_count2_t total_push_count = atomic_load_explicit(&queue->total_push_count, memory_order_relaxed);

    for (;;)
    {
        archi_lfqueue_count_t index = total_push_count & mask;

        archi_lfqueue_count_t push_count = atomic_load_explicit(&queue->push_count[index], memory_order_acquire);
        archi_lfqueue_count_t pop_count = atomic_load_explicit(&queue->pop_count[index], memory_order_relaxed);

        if (push_count != pop_count) // queue is full
            return false;

        archi_lfqueue_count_t revolution_count = total_push_count >> mask_bits;
        if (revolution_count == push_count) // current turn is ours
        {
            // Try to acquire the slot
            if (atomic_compare_exchange_weak_explicit(&queue->total_push_count,
                        &total_push_count, total_push_count + 1,
                        memory_order_relaxed, memory_order_relaxed))
            {
                if (queue->buffer != NULL)
                {
                    if (value != NULL)
                        memcpy((char*)queue->buffer + queue->element_size.padded * index, value, queue->element_size.actual);
                    else
                        memset((char*)queue->buffer + queue->element_size.padded * index, 0, queue->element_size.actual);
                }

                atomic_store_explicit(&queue->push_count[index], push_count + 1, memory_order_release);
                return true;
            }
        }
        else
            total_push_count = atomic_load_explicit(&queue->total_push_count, memory_order_relaxed);
    }
}

bool
archi_lfqueue_pop(
        archi_lfqueue_t queue,
        void *value)
{
    if (queue == NULL)
        return false;

    archi_lfqueue_count_t mask_bits = queue->mask_bits;
    archi_lfqueue_count_t mask = ((archi_lfqueue_count_t)1 << mask_bits) - 1;

    archi_lfqueue_count2_t total_pop_count = atomic_load_explicit(&queue->total_pop_count, memory_order_relaxed);

    for (;;)
    {
        archi_lfqueue_count_t index = total_pop_count & mask;

        archi_lfqueue_count_t pop_count = atomic_load_explicit(&queue->pop_count[index], memory_order_acquire);
        archi_lfqueue_count_t push_count = atomic_load_explicit(&queue->push_count[index], memory_order_relaxed);

        if (pop_count == push_count) // queue is empty
            return false;

        archi_lfqueue_count_t revolution_count = total_pop_count >> mask_bits;
        if (revolution_count == pop_count) // current turn is ours
        {
            // Try to acquire the slot
            if (atomic_compare_exchange_weak_explicit(&queue->total_pop_count,
                        &total_pop_count, total_pop_count + 1,
                        memory_order_relaxed, memory_order_relaxed))
            {
                if ((queue->buffer != NULL) && (value != NULL))
                    memcpy(value, (char*)queue->buffer + queue->element_size.padded * index, queue->element_size.actual);

                atomic_store_explicit(&queue->pop_count[index], pop_count + 1, memory_order_release);
                return true;
            }
        }
        else
            total_pop_count = atomic_load_explicit(&queue->total_pop_count, memory_order_relaxed);
    }
}

size_t
archi_lfqueue_capacity(
        archi_lfqueue_t queue)
{
    if (queue == NULL)
        return 0;

    return (size_t)1 << queue->mask_bits;
}

size_t
archi_lfqueue_element_size(
        archi_lfqueue_t queue)
{
    if (queue == NULL)
        return 0;

    return queue->element_size.actual;
}

