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
 * @brief Operations with thread groups and concurrent processing.
 */

#include "archi/thread/i/thread_group.fun.h"

#ifdef __STDC_NO_ATOMICS__
#  error Atomics are required, but not supported by the compiler.
#endif

#ifdef __STDC_NO_THREADS__
#  error Threads are required, but not supported by the compiler.
#endif

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

#  define CND_TIMEDWAIT(condvar, mutex, time_point) do {            \
    int res = cnd_timedwait(&(condvar), &(mutex), (time_point));    \
    assert((res == thrd_success) || (res == thrd_timedout));        \
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

#  define CND_TIMEDWAIT(condvar, mutex, time_point) do {    \
    cnd_timedwait(&(condvar), &(mutex), (time_point));      \
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

struct archi_thread_group {
    thrd_t *threads;
    size_t num_threads;

    struct {
        bool flag;
        bool sense;

        cnd_t cnd;
        mtx_t mtx;
    } ping, pong;

    atomic_flag busy; // whether threads are busy

    atomic_size_t num_work_items_done; // total number of processed work items
    atomic_size_t num_threads_done;    // number of threads that have finished processing

    struct archi_thread_group_dispatch dispatch; // current work task
};

/*****************************************************************************/

struct archi_thread_arg {
    archi_thread_group_t thread_group;
    size_t thread_idx;
};

static
int
archi_thread(
        void *arg)
{
    // Copy context and thread index
    archi_thread_group_t thread_group;
    size_t thread_idx;
    {
        struct archi_thread_arg *thread_arg = arg;
        assert(thread_arg != NULL);

        thread_group = thread_arg->thread_group;
        thread_idx = thread_arg->thread_idx;

        // Free the temporary argument object
        free(thread_arg);
    }

    // Initialize local flags
    bool ping_sense = false, pong_sense = false;

    // Loop until stop signal
    for (;;)
    {
        // Toggle flag sense
        ping_sense = !ping_sense;
        pong_sense = !pong_sense;

        struct archi_thread_group_dispatch dispatch;
        {
            MTX_LOCK(thread_group->ping.mtx);

            // Wait for a work task or stop signal
            while (thread_group->ping.flag != ping_sense)
                CND_WAIT(thread_group->ping.cnd, thread_group->ping.mtx);

            // Store a local copy of the dispatch
            dispatch = thread_group->dispatch;

            MTX_UNLOCK(thread_group->ping.mtx);
        }

        // Terminate on stop signal
        if (dispatch.work.function == NULL)
            return 0;

        // Acquire first work item
        size_t work_item_idx = atomic_fetch_add_explicit(&thread_group->num_work_items_done,
                dispatch.params.batch_size, memory_order_relaxed);
        size_t remaining_work_items = dispatch.params.batch_size;

        // Loop until no work items left
        while (work_item_idx < dispatch.params.size)
        {
            // Call the work function
            dispatch.work.function(dispatch.work.data, dispatch.params.offset + work_item_idx, thread_idx);
            remaining_work_items--;

            // Acquire next work item
            if (remaining_work_items > 0)
                work_item_idx++;
            else
            {
                work_item_idx = atomic_fetch_add_explicit(&thread_group->num_work_items_done,
                        dispatch.params.batch_size, memory_order_relaxed);
                remaining_work_items = dispatch.params.batch_size;
            }
        }

        // Check if the current thread is the last
        if (atomic_fetch_add_explicit(&thread_group->num_threads_done, 1,
                    memory_order_release) == thread_group->num_threads - 1)
        {
            atomic_thread_fence(memory_order_acquire); // synchronize memory writes from other threads

            // Call the callback function
            if (dispatch.callback.function != NULL)
                dispatch.callback.function(dispatch.callback.data,
                        dispatch.params.offset, dispatch.params.size, thread_idx);

            // Update pong flag
            {
                MTX_LOCK(thread_group->pong.mtx);

                thread_group->pong.flag = pong_sense;

                MTX_UNLOCK(thread_group->pong.mtx);
            }

            // Wake the master thread
            CND_BROADCAST(thread_group->pong.cnd);

            // Clear the busyness flag
            atomic_flag_clear_explicit(&thread_group->busy, memory_order_relaxed);
        }
    }
}

/*****************************************************************************/

archi_thread_group_t
archi_thread_group_create(
        archi_thread_group_start_params_t params,
        ARCHI_ERROR_PARAM_DECL)
{
    size_t thread_idx = 0;

    // Initialize threads context
    archi_thread_group_t thread_group = malloc(sizeof(*thread_group));
    if (thread_group == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate thread group context");
        goto failure;
    }

    *thread_group = (struct archi_thread_group){
        .num_threads = params.num_threads,
        .busy = ATOMIC_FLAG_INIT,
    };

    if (params.num_threads > 0)
    {
        // Create mutexes and condition variables
        int res;

        res = cnd_init(&thread_group->ping.cnd);
        if (res != thrd_success)
        {
            if (res == thrd_nomem)
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't initialize condition variable");
            else
                ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize condition variable");

            goto failure;
        }

        res = mtx_init(&thread_group->ping.mtx, mtx_plain);
        if (res != thrd_success)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize mutex");

            cnd_destroy(&thread_group->ping.cnd);
            goto failure;
        }

        res = cnd_init(&thread_group->pong.cnd);
        if (res != thrd_success)
        {
            if (res == thrd_nomem)
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't initialize condition variable");
            else
                ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize condition variable");

            cnd_destroy(&thread_group->ping.cnd);
            mtx_destroy(&thread_group->ping.mtx);
            goto failure;
        }

        res = mtx_init(&thread_group->pong.mtx, mtx_plain);
        if (res != thrd_success)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize mutex");

            cnd_destroy(&thread_group->ping.cnd);
            mtx_destroy(&thread_group->ping.mtx);
            cnd_destroy(&thread_group->pong.cnd);
            goto failure;
        }

        // Create threads
        thread_group->threads = malloc(sizeof(*thread_group->threads) * params.num_threads);
        if (thread_group->threads == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of threads [%zu]", params.num_threads);
            goto failure;
        }
    }

    for (; thread_idx < params.num_threads; thread_idx++)
    {
        struct archi_thread_arg *thread_arg = malloc(sizeof(*thread_arg));
        if (thread_arg == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate thread #%zu entry function argument", thread_idx);
            goto failure;
        }

        *thread_arg = (struct archi_thread_arg){.thread_group = thread_group, .thread_idx = thread_idx};

        int res = thrd_create(&thread_group->threads[thread_idx], archi_thread, thread_arg);
        if (res != thrd_success)
        {
            if (res == thrd_nomem)
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't create thread #%zu", thread_idx);
            else
                ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create thread #%zu", thread_idx);

            free(thread_arg);
            goto failure;
        }
    }

    ARCHI_ERROR_RESET();
    return thread_group;

failure:
    if (thread_group != NULL)
    {
        thread_group->num_threads = thread_idx; // number of created threads
        archi_thread_group_destroy(thread_group);
    }

    return NULL;
}

void
archi_thread_group_destroy(
        archi_thread_group_t thread_group)
{
    if (thread_group == NULL)
        return;

    // Wait while busy
    while (atomic_flag_test_and_set_explicit(&thread_group->busy, memory_order_relaxed))
        archi_thread_group_wait(thread_group);

    if (thread_group->num_threads > 0)
    {
        {
            MTX_LOCK(thread_group->ping.mtx);

            // Set stop signal
            thread_group->dispatch = (struct archi_thread_group_dispatch){0};

            // Update ping flag
            thread_group->ping.flag = !thread_group->ping.sense;

            MTX_UNLOCK(thread_group->ping.mtx);
        }

        // Wake slave threads
        CND_BROADCAST(thread_group->ping.cnd);
    }

    // Join threads and free memory
    for (size_t i = 0; i < thread_group->num_threads; i++)
        thrd_join(thread_group->threads[i], (int*)NULL);

    free(thread_group->threads);

    // Destroy mutexes, condition variables, and free memory
    if (thread_group->num_threads > 0)
    {
        cnd_destroy(&thread_group->ping.cnd);
        mtx_destroy(&thread_group->ping.mtx);
        cnd_destroy(&thread_group->pong.cnd);
        mtx_destroy(&thread_group->pong.mtx);
    }

    free(thread_group);
}

bool
archi_thread_group_dispatch(
        archi_thread_group_t thread_group,

        archi_thread_group_work_t work,
        archi_thread_group_callback_t callback,

        archi_thread_group_dispatch_params_t params,
        ARCHI_ERROR_PARAM_DECL)
{
    if (thread_group == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "thread group context is NULL");
        return false;
    }
    else if (work.function == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "thread group work function is NULL");
        return false;
    }
    else if (params.offset + params.size < params.offset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "thread group work offset+size overflows size_t");
        return false;
    }

    // Check if there is nothing to do
    if (params.size == 0)
    {
        ARCHI_ERROR_RESET();
        return true;
    }

    if (thread_group->num_threads > 0)
    {
        // Fail if slave threads are busy
        if (atomic_flag_test_and_set_explicit(&thread_group->busy, memory_order_relaxed))
        {
            ARCHI_ERROR_RESET();
            return false;
        }

        {
            MTX_LOCK(thread_group->ping.mtx);
            MTX_LOCK(thread_group->pong.mtx);

            // Calculate batch size if it's not specified
            if (params.batch_size == 0)
                params.batch_size = 1 + (params.size - 1) / thread_group->num_threads;

            // Assign the work
            thread_group->dispatch = (struct archi_thread_group_dispatch){
                .work = work,
                .callback = callback,
                .params = params,
            };

            // Initialize counters
            atomic_store_explicit(&thread_group->num_work_items_done, 0, memory_order_relaxed);
            atomic_store_explicit(&thread_group->num_threads_done, 0, memory_order_relaxed);

            // Toggle flag sense
            thread_group->ping.sense = !thread_group->ping.sense;
            thread_group->pong.sense = !thread_group->pong.sense;

            // Update ping flag
            thread_group->ping.flag = thread_group->ping.sense;

            MTX_UNLOCK(thread_group->pong.mtx);
            MTX_UNLOCK(thread_group->ping.mtx);
        }

        // Wake slave threads
        CND_BROADCAST(thread_group->ping.cnd);
    }
    else // do all the work in this thread
    {
        for (size_t work_item_idx = 0; work_item_idx < params.size; work_item_idx++)
            work.function(work.data, params.offset + work_item_idx, 0);

        if (callback.function != NULL)
            callback.function(callback.data, params.offset, params.size, 0);
    }

    ARCHI_ERROR_RESET();
    return true;
}

void
archi_thread_group_wait(
        archi_thread_group_t thread_group)
{
    if (thread_group == NULL)
        return;

    {
        MTX_LOCK(thread_group->pong.mtx);

        bool pong_sense = thread_group->pong.sense;
        while (thread_group->pong.flag != pong_sense)
            CND_WAIT(thread_group->pong.cnd, thread_group->pong.mtx);

        MTX_UNLOCK(thread_group->pong.mtx);
    }
}

void
archi_thread_group_wait_until(
        archi_thread_group_t thread_group,
        const struct timespec *time_point)
{
    if (thread_group == NULL)
        return;

    if (time_point == NULL)
    {
        archi_thread_group_wait(thread_group);
        return;
    }

    {
        MTX_LOCK(thread_group->pong.mtx);

        bool pong_sense = thread_group->pong.sense;
        while (thread_group->pong.flag != pong_sense)
            CND_TIMEDWAIT(thread_group->pong.cnd, thread_group->pong.mtx, time_point);

        MTX_UNLOCK(thread_group->pong.mtx);
    }
}

size_t
archi_thread_group_num_threads(
        archi_thread_group_t thread_group)
{
    if (thread_group == NULL)
        return 0;

    return thread_group->num_threads;
}

