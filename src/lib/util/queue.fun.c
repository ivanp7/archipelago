/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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

#include "archi/util/queue.fun.h"

#include <stdatomic.h>
#include <stdlib.h> // for aligned_alloc(), malloc(), free()
#include <string.h> // for memcpy()
#include <stdint.h> // for uint_fast*_t
#include <limits.h> // for CHAR_BIT

#ifdef ARCHI_FEATURE_QUEUE32

typedef uint_fast32_t archi_queue_count_t;
typedef uint_fast64_t archi_queue_count2_t;

typedef atomic_uint_fast32_t archi_queue_atomic_count_t;
typedef atomic_uint_fast64_t archi_queue_atomic_count2_t;

#else

typedef uint_fast16_t archi_queue_count_t;
typedef uint_fast32_t archi_queue_count2_t;

typedef atomic_uint_fast16_t archi_queue_atomic_count_t;
typedef atomic_uint_fast32_t archi_queue_atomic_count2_t;

#endif

struct archi_queue {
    char *buffer;

    struct {
        size_t full;
        size_t used;
    } element_size;

    archi_queue_count_t mask;
    unsigned char mask_bits;

    archi_queue_atomic_count_t *push_count, *pop_count;
    archi_queue_atomic_count2_t total_push_count, total_pop_count;
};

struct archi_queue*
archi_queue_alloc(
        size_t element_size,
        unsigned char element_alignment_log2,

        unsigned char capacity_log2)
{
    if (capacity_log2 > sizeof(archi_queue_count_t) * CHAR_BIT)
        return NULL;

    if ((element_size > 0) && (element_alignment_log2 >= sizeof(size_t) * CHAR_BIT))
        return NULL;

    struct archi_queue *queue = malloc(sizeof(*queue));
    if (queue == NULL)
        return NULL;

    size_t capacity = (size_t)1 << capacity_log2;

    if (element_size > 0)
    {
        size_t element_alignment = (size_t)1 << element_alignment_log2;
        size_t element_size_full = (element_size + (element_alignment - 1)) & ~(element_alignment - 1);

        size_t memory_size = element_size_full * capacity;
        if (memory_size / capacity != element_size_full) // overflow
        {
            free(queue);
            return NULL;
        }

        queue->buffer = aligned_alloc(element_alignment, memory_size);
        if (queue->buffer == NULL)
        {
            free(queue);
            return NULL;
        }

        queue->element_size.full = element_size_full;
        queue->element_size.used = element_size;
    }
    else
    {
        queue->buffer = NULL;
        queue->element_size.full = 0;
        queue->element_size.used = 0;
    }

    queue->mask = capacity - 1;
    queue->mask_bits = capacity_log2;

    {
        size_t memory_size = sizeof(*queue->push_count) * capacity;

        if (memory_size / capacity != sizeof(*queue->push_count))
        {
            free(queue->buffer);
            free(queue);
            return NULL;
        }

        queue->push_count = malloc(memory_size);
        if (queue->push_count == NULL)
        {
            free(queue->buffer);
            free(queue);
            return NULL;
        }

        queue->pop_count = malloc(memory_size);
        if (queue->pop_count == NULL)
        {
            free(queue->push_count);
            free(queue->buffer);
            free(queue);
            return NULL;
        }
    }

    for (size_t i = 0; i < capacity; i++)
    {
        atomic_init(&queue->push_count[i], 0);
        atomic_init(&queue->pop_count[i], 0);
    }

    atomic_init(&queue->total_push_count, 0);
    atomic_init(&queue->total_pop_count, 0);

    return queue;
}

void
archi_queue_free(
        struct archi_queue *queue)
{
    if (queue != NULL)
    {
        free(queue->push_count);
        free(queue->pop_count);
        free(queue->buffer);

        free(queue);
    }
}

bool
archi_queue_push(
        struct archi_queue *queue,
        const void *value)
{
    if (queue == NULL)
        return false;

    archi_queue_count_t mask = queue->mask;

    unsigned char mask_bits = queue->mask_bits;

    archi_queue_count2_t total_push_count = atomic_load_explicit(&queue->total_push_count, memory_order_relaxed);

    for (;;)
    {
        archi_queue_count_t index = total_push_count & mask;

        archi_queue_count_t push_count = atomic_load_explicit(&queue->push_count[index], memory_order_acquire);
        archi_queue_count_t pop_count = atomic_load_explicit(&queue->pop_count[index], memory_order_relaxed);

        if (push_count != pop_count) // queue is full
            return false;

        archi_queue_count_t revolution_count = total_push_count >> mask_bits;
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
                        memcpy(queue->buffer + queue->element_size.full * index, value, queue->element_size.used);
                    else
                        memset(queue->buffer + queue->element_size.full * index, 0, queue->element_size.used);
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
archi_queue_pop(
        struct archi_queue *queue,
        void *value)
{
    if (queue == NULL)
        return false;

    archi_queue_count_t mask = queue->mask;

    unsigned char mask_bits = queue->mask_bits;

    archi_queue_count2_t total_pop_count = atomic_load_explicit(&queue->total_pop_count, memory_order_relaxed);

    for (;;)
    {
        archi_queue_count_t index = total_pop_count & mask;

        archi_queue_count_t pop_count = atomic_load_explicit(&queue->pop_count[index], memory_order_acquire);
        archi_queue_count_t push_count = atomic_load_explicit(&queue->push_count[index], memory_order_relaxed);

        if (pop_count == push_count) // queue is empty
            return false;

        archi_queue_count_t revolution_count = total_pop_count >> mask_bits;
        if (revolution_count == pop_count) // current turn is ours
        {
            // Try to acquire the slot
            if (atomic_compare_exchange_weak_explicit(&queue->total_pop_count,
                        &total_pop_count, total_pop_count + 1,
                        memory_order_relaxed, memory_order_relaxed))
            {
                if ((queue->buffer != NULL) && (value != NULL))
                    memcpy(value, queue->buffer + queue->element_size.full * index, queue->element_size.used);

                atomic_store_explicit(&queue->pop_count[index], pop_count + 1, memory_order_release);
                return true;
            }
        }
        else
            total_pop_count = atomic_load_explicit(&queue->total_pop_count, memory_order_relaxed);
    }
}

size_t
archi_queue_capacity(
        struct archi_queue *queue)
{
    if (queue == NULL)
        return 0;

    return (size_t)queue->mask + 1;
}

size_t
archi_queue_element_size(
        struct archi_queue *queue)
{
    if (queue == NULL)
        return 0;

    return queue->element_size.used;
}

