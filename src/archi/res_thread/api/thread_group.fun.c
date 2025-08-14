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
 * @brief Operations with thread groups and concurrent processing.
 */

#include "archi/res_thread/api/thread_group.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <stdatomic.h> // for atomic_* functions and types
#include <threads.h> // for thrd_* functions and types
#include <stdbool.h>
#include <assert.h>

#ifndef NDEBUG

#  define MTX_LOCK(mutex) do {                  \
    int res = mtx_lock(&(mutex));               \
    assert(res == thrd_success);                \
} while (0)

#  define CND_WAIT(condvar, mutex) do {         \
    int res = cnd_wait(&(condvar), &(mutex));   \
    assert(res == thrd_success);                \
} while (0)

#  define CND_BROADCAST(condvar) do {           \
    int res = cnd_broadcast(&(condvar));        \
    assert(res == thrd_success);                \
} while (0)

#  define MTX_UNLOCK(mutex) do {                \
    int res = mtx_unlock(&(mutex));             \
    assert(res == thrd_success);                \
} while (0)

#else

#  define MTX_LOCK(mutex) do {          \
    mtx_lock(&(mutex));                 \
} while (0)

#  define CND_WAIT(condvar, mutex) do { \
    cnd_wait(&(condvar), &(mutex));     \
} while (0)

#  define CND_BROADCAST(condvar) do {   \
    cnd_broadcast(&(condvar));          \
} while (0)

#  define MTX_UNLOCK(mutex) do {        \
    mtx_unlock(&(mutex));               \
} while (0)

#endif

/*****************************************************************************/

struct archi_thread_group_dispatch {
    archi_thread_group_work_t work;
    archi_thread_group_callback_t callback;
    archi_thread_group_dispatch_params_t params;
};

struct archi_thread_group_context {
    thrd_t *threads;
    archi_thread_group_start_params_t params;

    struct {
        bool flag;
        bool sense;

        cnd_t cnd;
        mtx_t mtx;
    } ping, pong;

    atomic_size_t num_work_items_done; // total number of processed work items
    atomic_size_t num_threads_done;    // number of threads that have finished processing

    struct archi_thread_group_dispatch dispatch; // current work task
};

/*****************************************************************************/

struct archi_thread_arg {
    archi_thread_group_context_t context;
    size_t thread_idx;
};

static
int
archi_thread(
        void *arg)
{
    // Copy context and thread index
    archi_thread_group_context_t context;
    size_t thread_idx;
    {
        struct archi_thread_arg *thread_arg = arg;
        assert(thread_arg != NULL);

        context = thread_arg->context;
        thread_idx = thread_arg->thread_idx;

        // Free the temporary argument object
        free(thread_arg);
    }

    // Initialize local data
    const size_t thread_counter_last = context->params.num_threads - 1;

    bool ping_sense = false, pong_sense = false;
    struct archi_thread_group_dispatch dispatch;

    // Loop until stop signal
    for (;;)
    {
        // Toggle flag sense
        ping_sense = !ping_sense;
        pong_sense = !pong_sense;

        // Wait for flag
        {
            MTX_LOCK(context->ping.mtx);

            while (context->ping.flag != ping_sense)
            {
                CND_WAIT(context->ping.cnd, context->ping.mtx);
            }
            dispatch = context->dispatch;

            MTX_UNLOCK(context->ping.mtx);
        }

        // Terminate on stop signal
        if (dispatch.work.function == NULL)
            break;

        // Acquire first work item
        size_t work_item_idx = atomic_fetch_add_explicit(&context->num_work_items_done,
                dispatch.params.batch_size, memory_order_relaxed);
        size_t remaining_work_items = dispatch.params.batch_size;

        // Loop until no work items left
        while (work_item_idx < dispatch.work.size)
        {
            // Call the work function
            dispatch.work.function(dispatch.work.data, work_item_idx, thread_idx);
            remaining_work_items--;

            // Acquire next work item
            if (remaining_work_items > 0)
                work_item_idx++;
            else
            {
                work_item_idx = atomic_fetch_add_explicit(&context->num_work_items_done,
                        dispatch.params.batch_size, memory_order_relaxed);
                remaining_work_items = dispatch.params.batch_size;
            }
        }

        // Check if the current thread is the last
        if (atomic_fetch_add_explicit(&context->num_threads_done, 1,
                    memory_order_release) == thread_counter_last)
        {
            atomic_thread_fence(memory_order_acquire); // synchronize memory writes from other threads

            if (dispatch.callback.function != NULL)
                dispatch.callback.function(dispatch.callback.data, dispatch.work.size, thread_idx);
            else // wake master thread
            {
                MTX_LOCK(context->pong.mtx);

                context->pong.flag = pong_sense;
                CND_BROADCAST(context->pong.cnd);

                MTX_UNLOCK(context->pong.mtx);
            }
        }
    }

    return 0;
}

/*****************************************************************************/

archi_thread_group_context_t
archi_thread_group_start(
        archi_thread_group_start_params_t params,
        archi_status_t *code)
{
    archi_status_t status = 0;

    size_t thread_idx = 0;

    // Initialize threads context
    archi_thread_group_context_t context = malloc(sizeof(*context));
    if (context == NULL)
    {
        status = ARCHI_STATUS_ENOMEMORY;
        goto failure;
    }

    *context = (struct archi_thread_group_context){
        .params = params,
    };

    if (params.num_threads > 0)
    {
        // Create mutexes and condition variables
        int res;

        res = cnd_init(&context->ping.cnd);
        if (res != thrd_success)
        {
            if (res == thrd_nomem)
                status = ARCHI_STATUS_ENOMEMORY;
            else
                status = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }

        res = mtx_init(&context->ping.mtx, mtx_plain);
        if (res != thrd_success)
        {
            cnd_destroy(&context->ping.cnd);

            status = ARCHI_STATUS_ERESOURCE;
            goto failure;
        }

        res = cnd_init(&context->pong.cnd);
        if (res != thrd_success)
        {
            cnd_destroy(&context->ping.cnd);
            mtx_destroy(&context->ping.mtx);

            if (res == thrd_nomem)
                status = ARCHI_STATUS_ENOMEMORY;
            else
                status = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }

        res = mtx_init(&context->pong.mtx, mtx_plain);
        if (res != thrd_success)
        {
            cnd_destroy(&context->ping.cnd);
            mtx_destroy(&context->ping.mtx);
            cnd_destroy(&context->pong.cnd);

            status = ARCHI_STATUS_ERESOURCE;
            goto failure;
        }

        // Create threads
        context->threads = malloc(sizeof(*context->threads) * params.num_threads);
        if (context->threads == NULL)
        {
            status = ARCHI_STATUS_ENOMEMORY;
            goto failure;
        }
    }

    for (; thread_idx < params.num_threads; thread_idx++)
    {
        struct archi_thread_arg *thread_arg = malloc(sizeof(*thread_arg));
        if (thread_arg == NULL)
        {
            status = ARCHI_STATUS_ENOMEMORY;
            goto failure;
        }

        *thread_arg = (struct archi_thread_arg){.context = context, .thread_idx = thread_idx};

        int res = thrd_create(&context->threads[thread_idx], archi_thread, thread_arg);
        if (res != thrd_success)
        {
            free(thread_arg);

            if (res == thrd_nomem)
                status = ARCHI_STATUS_ENOMEMORY;
            else
                status = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }
    }

    if (code != NULL)
        *code = status;

    return context;

failure:
    if (context != NULL)
    {
        context->params.num_threads = thread_idx; // number of created threads
        archi_thread_group_stop(context);
    }

    if (code != NULL)
        *code = status;

    return NULL;
}

void
archi_thread_group_stop(
        archi_thread_group_context_t context)
{
    if (context == NULL)
        return;

    // Wake threads to terminate them
    if (context->params.num_threads > 0)
    {
        MTX_LOCK(context->ping.mtx);

        context->dispatch = (struct archi_thread_group_dispatch){0}; // stop signal

        context->ping.flag = !context->ping.sense;
        CND_BROADCAST(context->ping.cnd);

        MTX_UNLOCK(context->ping.mtx);
    }

    // Join threads and free memory
    for (size_t i = 0; i < context->params.num_threads; i++)
        thrd_join(context->threads[i], (int*)NULL);

    free(context->threads);

    // Destroy mutexes, condition variables, and free memory
    if (context->params.num_threads > 0)
    {
        cnd_destroy(&context->ping.cnd);
        mtx_destroy(&context->ping.mtx);
        cnd_destroy(&context->pong.cnd);
        mtx_destroy(&context->pong.mtx);
    }

    free(context);
}

archi_status_t
archi_thread_group_dispatch(
        archi_thread_group_context_t context,

        archi_thread_group_work_t work,
        archi_thread_group_callback_t callback,
        archi_thread_group_dispatch_params_t params)
{
    if ((context == NULL) || (work.function == NULL))
        return ARCHI_STATUS_EMISUSE;

    if (work.size == 0)
        return 0;

    if (context->params.num_threads > 0)
    {
        // Calculate batch size if it's not specified
        if (params.batch_size == 0)
            params.batch_size = (work.size - 1) / context->params.num_threads + 1;

        // Assign the work, initialize counters and flags, wake slave threads
        {
            MTX_LOCK(context->ping.mtx);

            context->dispatch = (struct archi_thread_group_dispatch){
                .work = work,
                .callback = callback,
                .params = params,
            };

            atomic_store_explicit(&context->num_work_items_done, 0, memory_order_relaxed);
            atomic_store_explicit(&context->num_threads_done, 0, memory_order_relaxed);

            context->ping.flag = context->ping.sense = !context->ping.sense;
            CND_BROADCAST(context->ping.cnd);

            MTX_UNLOCK(context->ping.mtx);
        }

        // Wait until the work is done if no callback function was specified
        if (callback.function == NULL)
        {
            MTX_LOCK(context->pong.mtx);

            bool pong_sense = context->pong.sense = !context->pong.sense;

            while (context->pong.flag != pong_sense)
            {
                CND_WAIT(context->pong.cnd, context->pong.mtx);
            }

            MTX_UNLOCK(context->pong.mtx);
        }
    }
    else // do all the work in this thread
    {
        for (size_t work_item_idx = 0; work_item_idx < work.size; work_item_idx++)
            work.function(work.data, work_item_idx, 0);

        if (callback.function != NULL)
            callback.function(callback.data, work.size, 0);
    }

    return 0;
}

archi_thread_group_start_params_t
archi_thread_group_parameters(
        archi_thread_group_context_t context)
{
    if (context == NULL)
        return (archi_thread_group_start_params_t){0};

    return context->params;
}

