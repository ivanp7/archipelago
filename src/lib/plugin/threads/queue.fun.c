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

#include "archi/plugin/threads/queue.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <stdatomic.h>
#include <stdlib.h> // for aligned_alloc(), malloc(), free()
#include <string.h> // for strcmp(), memcpy()
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

    archi_queue_count_t mask_bits;

    archi_queue_atomic_count_t *push_count, *pop_count;
    archi_queue_atomic_count2_t total_push_count, total_pop_count;
};

struct archi_queue*
archi_queue_alloc(
        archi_queue_config_t config)
{
    if (config.capacity_log2 > sizeof(archi_queue_count_t) * CHAR_BIT)
        return NULL;

    if ((config.element_size > 0) && (config.element_alignment_log2 >= sizeof(size_t) * CHAR_BIT))
        return NULL;

    struct archi_queue *queue = malloc(sizeof(*queue));
    if (queue == NULL)
        return NULL;

    size_t capacity = (size_t)1 << config.capacity_log2;

    if (config.element_size > 0)
    {
        size_t element_alignment = (size_t)1 << config.element_alignment_log2;
        size_t element_size_full = (config.element_size + (element_alignment - 1)) & ~(element_alignment - 1);

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
        queue->element_size.used = config.element_size;
    }
    else
    {
        queue->buffer = NULL;
        queue->element_size.full = 0;
        queue->element_size.used = 0;
    }

    queue->mask_bits = config.capacity_log2;

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

    archi_queue_count_t mask_bits = queue->mask_bits;
    archi_queue_count_t mask = ((archi_queue_count_t)1 << mask_bits) - 1;

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

    archi_queue_count_t mask_bits = queue->mask_bits;
    archi_queue_count_t mask = ((archi_queue_count_t)1 << mask_bits) - 1;

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

    return (size_t)1 << queue->mask_bits;
}

size_t
archi_queue_element_size(
        struct archi_queue *queue)
{
    if (queue == NULL)
        return 0;

    return queue->element_size.used;
}

/*****************************************************************************/

static
ARCHI_CONTAINER_ELEMENT_FUNC(archi_queue_context_init_config)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_value_t *value = element;
    archi_queue_config_t *config = data;

    if (strcmp(key, ARCHI_QUEUE_CONFIG_KEY) == 0)
    {
        if ((value->type != ARCHI_VALUE_DATA) || (value->ptr == NULL) ||
                (value->size != sizeof(*config)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value->ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(key, ARCHI_QUEUE_CONFIG_KEY_CAPACITY_LOG2) == 0)
    {
        if ((value->type != ARCHI_VALUE_UINT) || (value->ptr == NULL) ||
                (value->size != sizeof(config->capacity_log2)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->capacity_log2 = *(size_t*)value->ptr;
        return 0;
    }
    else if (strcmp(key, ARCHI_QUEUE_CONFIG_KEY_ELEMENT_ALIGNMENT_LOG2) == 0)
    {
        if ((value->type != ARCHI_VALUE_UINT) || (value->ptr == NULL) ||
                (value->size != sizeof(config->element_alignment_log2)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->element_alignment_log2 = *(size_t*)value->ptr;
        return 0;
    }
    else if (strcmp(key, ARCHI_QUEUE_CONFIG_KEY_ELEMENT_SIZE) == 0)
    {
        if ((value->type != ARCHI_VALUE_UINT) || (value->ptr == NULL) ||
                (value->size != sizeof(config->element_size)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->element_size = *(size_t*)value->ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_queue_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_queue_config_t queue_config = {0};
    if (config.data != NULL)
    {
        code = archi_container_traverse(config, archi_queue_context_init_config, &queue_config);
        if (code != 0)
            return code;
    }

    struct archi_queue *queue_context = archi_queue_alloc(queue_config);
    if (queue_context == NULL)
        return ARCHI_ERROR_ALLOC;

    *context = queue_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_queue_context_final)
{
    archi_queue_free(context);
}

const archi_context_interface_t archi_queue_context_interface = {
    .init_fn = archi_queue_context_init,
    .final_fn = archi_queue_context_final,
};

