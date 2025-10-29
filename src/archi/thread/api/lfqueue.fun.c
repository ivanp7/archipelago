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
 * @brief Lock-free queue operations.
 */

#include "archi/thread/api/lfqueue.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy(), memset()

#ifdef __STDC_NO_ATOMICS__
#  error Atomics are required, but not supported by the compiler.
#endif

#include <stdatomic.h> // for atomic_uint_fast*_t
#include <stdint.h> // for uint_fast*_t
#include <limits.h> // for CHAR_BIT

#ifdef ARCHI_FEATURE_LFQUEUE32

typedef uint_fast32_t archi_thread_lfqueue_count_t;
typedef uint_fast64_t archi_thread_lfqueue_count2_t;

typedef atomic_uint_fast32_t archi_thread_lfqueue_atomic_count_t;
typedef atomic_uint_fast64_t archi_thread_lfqueue_atomic_count2_t;

#else

typedef uint_fast16_t archi_thread_lfqueue_count_t;
typedef uint_fast32_t archi_thread_lfqueue_count2_t;

typedef atomic_uint_fast16_t archi_thread_lfqueue_atomic_count_t;
typedef atomic_uint_fast32_t archi_thread_lfqueue_atomic_count2_t;

#endif

struct archi_thread_lfqueue {
    void *buffer;

    archi_thread_lfqueue_alloc_params_t params;
    unsigned int mask_bits;

    archi_thread_lfqueue_atomic_count_t *push_count, *pop_count;
    archi_thread_lfqueue_atomic_count2_t total_push_count, total_pop_count;
};

archi_thread_lfqueue_t
archi_thread_lfqueue_alloc(
        archi_thread_lfqueue_alloc_params_t params,
        ARCHI_ERROR_PARAMETER_DECL)
{
    // Validate input parameters
    if (params.capacity == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "queue capacity is zero");
        return NULL;
    }
    else if ((params.capacity & (params.capacity - 1)) != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "queue capacity (%zu) is not a power of two", params.capacity);
        return NULL;
    }
    else if ((params.capacity - 1) > (archi_thread_lfqueue_count_t)-1) // strictly greater
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "queue capacity (%zu) is bigger than supported", params.capacity);
        return NULL;
    }

    if (params.element.size != 0)
    {
        if (params.element.alignment == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "queue element alignment requirement is zero");
            return NULL;
        }
        else if ((params.element.alignment & (params.element.alignment - 1)) != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "queue element alignment requirement (%#zx) is not a power of two",
                    params.element.alignment);
            return NULL;
        }
        else if (params.element.size % params.element.alignment != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "queue element size (%zu) isn't a multiple of alignment requirement (%#zx)",
                    params.element.size, params.element.alignment);
            return NULL;
        }
        else if (ARCHI_SIZE_OVERFLOW(params.capacity, params.element.size))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "queue buffer size (%zu * %zu) doesn't fit into size_t",
                    params.capacity, params.element.size);
            return NULL;
        }
    }

    // Calculate data buffer size
    size_t buffer_size = params.capacity * params.element.size;

    // Allocate the queue object
    archi_thread_lfqueue_t queue = malloc(sizeof(*queue));
    if (queue == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate lock-free queue");
        return NULL;
    }

    // Allocate the data buffer
    void *buffer = NULL;

    if (buffer_size != 0)
    {
        buffer = aligned_alloc(params.element.alignment, buffer_size);
        if (buffer == NULL)
        {
            free(queue);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate buffer of lock-free queue (%zu bytes, alignment = %#zx)",
                    buffer_size, params.element.alignment);
            return NULL;
        }
    }

    // Calculate number of index mask bits
    unsigned int mask_bits = 0;
    for (size_t c = params.capacity; c != 1; c >>= 1, mask_bits++);

    // Initialize the queue object
    *queue = (struct archi_thread_lfqueue){
        .buffer = buffer,
        .params = params,
        .mask_bits = mask_bits,
    };

    // Allocate the arrays of counters
    buffer_size = sizeof(*queue->push_count) * params.capacity;

    queue->push_count = malloc(buffer_size);
    if (queue->push_count == NULL)
    {
        free(queue->buffer);
        free(queue);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of push counters of lock-free queue (%zu bytes)",
                buffer_size);
        return NULL;
    }

    queue->pop_count = malloc(buffer_size);
    if (queue->pop_count == NULL)
    {
        free(queue->push_count);
        free(queue->buffer);
        free(queue);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of pop counters of lock-free queue (%zu bytes)",
                buffer_size);
        return NULL;
    }

    // Initialize all the counters
    for (size_t i = 0; i < params.capacity; i++)
    {
        atomic_init(&queue->push_count[i], 0);
        atomic_init(&queue->pop_count[i], 0);
    }

    atomic_init(&queue->total_push_count, 0);
    atomic_init(&queue->total_pop_count, 0);

    ARCHI_ERROR_RESET();
    return queue;
}

void
archi_thread_lfqueue_free(
        archi_thread_lfqueue_t queue)
{
    if (queue == NULL)
        return;

    free(queue->push_count);
    free(queue->pop_count);
    free(queue->buffer);
    free(queue);
}

bool
archi_thread_lfqueue_push(
        archi_thread_lfqueue_t queue,
        const void *value)
{
    if (queue == NULL)
        return false;

    unsigned int mask_bits = queue->mask_bits;
    archi_thread_lfqueue_count_t mask = queue->params.capacity - 1;

    archi_thread_lfqueue_count2_t total_push_count =
        atomic_load_explicit(&queue->total_push_count, memory_order_relaxed);

    for (;;)
    {
        archi_thread_lfqueue_count_t index = total_push_count & mask;

        archi_thread_lfqueue_count_t push_count =
            atomic_load_explicit(&queue->push_count[index], memory_order_acquire);
        archi_thread_lfqueue_count_t pop_count =
            atomic_load_explicit(&queue->pop_count[index], memory_order_relaxed);

        if (push_count != pop_count) // queue is full
            return false;

        archi_thread_lfqueue_count_t revolution_count = total_push_count >> mask_bits;
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
                        memcpy((char*)queue->buffer + queue->params.element.size * index,
                                value, queue->params.element.size);
                    else
                        memset((char*)queue->buffer + queue->params.element.size * index,
                                0, queue->params.element.size);
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
archi_thread_lfqueue_pop(
        archi_thread_lfqueue_t queue,
        void *value)
{
    if (queue == NULL)
        return false;

    unsigned int mask_bits = queue->mask_bits;
    archi_thread_lfqueue_count_t mask = queue->params.capacity - 1;

    archi_thread_lfqueue_count2_t total_pop_count =
        atomic_load_explicit(&queue->total_pop_count, memory_order_relaxed);

    for (;;)
    {
        archi_thread_lfqueue_count_t index = total_pop_count & mask;

        archi_thread_lfqueue_count_t pop_count =
            atomic_load_explicit(&queue->pop_count[index], memory_order_acquire);
        archi_thread_lfqueue_count_t push_count =
            atomic_load_explicit(&queue->push_count[index], memory_order_relaxed);

        if (pop_count == push_count) // queue is empty
            return false;

        archi_thread_lfqueue_count_t revolution_count = total_pop_count >> mask_bits;
        if (revolution_count == pop_count) // current turn is ours
        {
            // Try to acquire the slot
            if (atomic_compare_exchange_weak_explicit(&queue->total_pop_count,
                        &total_pop_count, total_pop_count + 1,
                        memory_order_relaxed, memory_order_relaxed))
            {
                if ((queue->buffer != NULL) && (value != NULL))
                    memcpy(value, (char*)queue->buffer + queue->params.element.size * index,
                            queue->params.element.size);

                atomic_store_explicit(&queue->pop_count[index], pop_count + 1, memory_order_release);
                return true;
            }
        }
        else
            total_pop_count = atomic_load_explicit(&queue->total_pop_count, memory_order_relaxed);
    }
}

size_t
archi_thread_lfqueue_capacity(
        archi_thread_lfqueue_t queue)
{
    if (queue == NULL)
        return 0;

    return queue->params.capacity;
}

size_t
archi_thread_lfqueue_element_size(
        archi_thread_lfqueue_t queue)
{
    if (queue == NULL)
        return 0;

    return queue->params.element.size;
}

size_t
archi_thread_lfqueue_element_alignment(
        archi_thread_lfqueue_t queue)
{
    if (queue == NULL)
        return 0;

    return queue->params.element.alignment;
}

