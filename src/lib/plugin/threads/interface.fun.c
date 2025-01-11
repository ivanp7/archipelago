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
 * @brief Operations with threads and concurrent processing.
 */

#include "archi/plugin/threads/interface.fun.h"
#include "archi/util/error.def.h"

#include <threads.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <assert.h>

struct archi_threads_context {
    struct {
        thrd_t *threads;
        size_t num_threads;

        atomic_flag busy;

        atomic_bool ping_flag;
        atomic_bool pong_flag;
        bool ping_sense;
        bool pong_sense;

        bool use_ping_cnd;
        cnd_t ping_cnd;
        mtx_t ping_mtx;

        cnd_t pong_cnd;
        mtx_t pong_mtx;
    } persistent;

    struct {
        archi_threads_job_t job;
        archi_threads_callback_t callback;

        size_t batch_size;
        bool use_pong_cnd;

        atomic_uint done_tasks;
        atomic_ushort thread_counter;
    } current;

    bool terminate;
};

struct archi_thread_arg {
    struct archi_threads_context *context;
    size_t thread_idx;
};

static
int
archi_thread(
        void *arg)
{
    struct archi_threads_context *context;
    size_t thread_idx;
    {
        struct archi_thread_arg *thread_arg = arg;
        assert(thread_arg != NULL);

        context = thread_arg->context;
        thread_idx = thread_arg->thread_idx;

        // Free a temporary object
        free(thread_arg);
    }

    size_t thread_counter_last = context->persistent.num_threads - 1;
    bool use_ping_cnd = context->persistent.use_ping_cnd;

    bool ping_sense = false, pong_sense = false;

    for (;;)
    {
        ping_sense = !ping_sense;
        pong_sense = !pong_sense;

        if (use_ping_cnd)
        {
#ifndef NDEBUG
            int res =
#endif
                mtx_lock(&context->persistent.ping_mtx);
            assert(res == thrd_success);
        }

        // Wait until signal
        while (atomic_load_explicit(&context->persistent.ping_flag,
                    memory_order_acquire) != ping_sense)
        {
            if (use_ping_cnd)
            {
#ifndef NDEBUG
                int res =
#endif
                    cnd_wait(&context->persistent.ping_cnd,
                            &context->persistent.ping_mtx);
                assert(res == thrd_success);
            }
        }

        if (use_ping_cnd)
        {
#ifndef NDEBUG
            int res =
#endif
                mtx_unlock(&context->persistent.ping_mtx);
            assert(res == thrd_success);
        }

        if (context->terminate)
            break;

        // Copy the current job
        archi_threads_job_t job = context->current.job;
        size_t batch_size = context->current.batch_size;

        // Acquire first task
        size_t task_idx = atomic_fetch_add_explicit(
                &context->current.done_tasks, batch_size, memory_order_relaxed);
        size_t remaining_tasks = batch_size;

        // Loop until no subtasks left
        while (task_idx < job.num_tasks)
        {
            // Execute concurrent processing function
            job.function(job.data, task_idx, thread_idx);
            remaining_tasks--;

            // Acquire next task
            if (remaining_tasks > 0)
                task_idx++;
            else
            {
                task_idx = atomic_fetch_add_explicit(
                        &context->current.done_tasks, batch_size, memory_order_relaxed);
                remaining_tasks = batch_size;
            }
        }

        // Check if the current thread is the last
        if (atomic_fetch_add_explicit(&context->current.thread_counter, 1,
                    memory_order_acq_rel) == thread_counter_last)
        {
            // Wake master thread or execute callback function
            atomic_store_explicit(&context->persistent.pong_flag,
                    pong_sense, memory_order_release);

            if (context->current.callback.function != NULL)
                context->current.callback.function(
                        context->current.callback.data, job.num_tasks, thread_idx);
            else if (context->current.use_pong_cnd)
            {
                {
#ifndef NDEBUG
                    int res =
#endif
                        mtx_lock(&context->persistent.pong_mtx);
                    assert(res == thrd_success);
                }
                cnd_broadcast(&context->persistent.pong_cnd);
                {
#ifndef NDEBUG
                    int res =
#endif
                        mtx_unlock(&context->persistent.pong_mtx);
                    assert(res == thrd_success);
                }
            }

            // Threads are no longer busy
            atomic_flag_clear_explicit(&context->persistent.busy, memory_order_release);
        }
    }

    return 0;
}

struct archi_threads_context*
archi_threads_start(
        archi_threads_config_t config,

        archi_status_t *code)
{
    archi_status_t status = 0;

    size_t thread_idx = 0;

    // Initialize threads context
    struct archi_threads_context *context = malloc(sizeof(*context));
    if (context == NULL)
    {
        status = ARCHI_ERROR_ALLOC;
        goto failure;
    }

    *context = (struct archi_threads_context){.persistent = {
        .num_threads = config.num_threads,
        .busy = ATOMIC_FLAG_INIT,
        .use_ping_cnd = !config.busy_wait,
    }};

    // Create mutexes and condition variables
    if (context->persistent.use_ping_cnd)
    {
        int res;

        res = cnd_init(&context->persistent.ping_cnd);
        if (res != thrd_success)
        {
            if (res == thrd_nomem)
                status = ARCHI_ERROR_ALLOC;
            else
                status = ARCHI_ERROR_UNKNOWN;

            goto failure;
        }

        res = mtx_init(&context->persistent.ping_mtx, mtx_plain);
        if (res != thrd_success)
        {
            cnd_destroy(&context->persistent.ping_cnd);

            status = ARCHI_ERROR_UNKNOWN;
            goto failure;
        }
    }

    {
        int res;

        res = cnd_init(&context->persistent.pong_cnd);
        if (res != thrd_success)
        {
            cnd_destroy(&context->persistent.ping_cnd);
            mtx_destroy(&context->persistent.ping_mtx);

            if (res == thrd_nomem)
                status = ARCHI_ERROR_ALLOC;
            else
                status = ARCHI_ERROR_UNKNOWN;

            goto failure;
        }

        res = mtx_init(&context->persistent.pong_mtx, mtx_plain);
        if (res != thrd_success)
        {
            cnd_destroy(&context->persistent.ping_cnd);
            mtx_destroy(&context->persistent.ping_mtx);
            cnd_destroy(&context->persistent.pong_cnd);

            status = ARCHI_ERROR_UNKNOWN;
            goto failure;
        }
    }

    // Create threads
    if (config.num_threads > 0)
    {
        context->persistent.threads = malloc(sizeof(*context->persistent.threads) * config.num_threads);
        if (context->persistent.threads == NULL)
        {
            status = ARCHI_ERROR_ALLOC;
            goto failure;
        }
    }

    for (thread_idx = 0; thread_idx < config.num_threads; thread_idx++)
    {
        struct archi_thread_arg *thread_arg = malloc(sizeof(*thread_arg));
        if (thread_arg == NULL)
        {
            status = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        *thread_arg = (struct archi_thread_arg){.context = context, .thread_idx = thread_idx};

        int res = thrd_create(&context->persistent.threads[thread_idx], archi_thread, thread_arg);
        if (res != thrd_success)
        {
            free(thread_arg);

            if (res == thrd_nomem)
                status = ARCHI_ERROR_ALLOC;
            else
                status = ARCHI_ERROR_UNKNOWN;

            goto failure;
        }
    }

    if (code != NULL)
        *code = status;

    return context;

failure:
    if (context != NULL)
    {
        context->persistent.num_threads = thread_idx; // number of created threads
        archi_threads_stop(context);
    }

    if (code != NULL)
        *code = status;

    return NULL;
}

void
archi_threads_stop(
        struct archi_threads_context *context)
{
    if (context == NULL)
        return;

    // Wake threads
    context->terminate = true;

    atomic_store_explicit(&context->persistent.ping_flag,
            !context->persistent.ping_sense, memory_order_release);

    if (context->persistent.use_ping_cnd)
    {
        {
#ifndef NDEBUG
            int res =
#endif
                mtx_lock(&context->persistent.ping_mtx);
            assert(res == thrd_success);
        }
        cnd_broadcast(&context->persistent.ping_cnd);
        {
#ifndef NDEBUG
            int res =
#endif
                mtx_unlock(&context->persistent.ping_mtx);
            assert(res == thrd_success);
        }
    }

    // Join threads
    for (size_t i = 0; i < context->persistent.num_threads; i++)
        thrd_join(context->persistent.threads[i], (int*)NULL);

    // Destroy mutexes, condition variables, and free memory
    free(context->persistent.threads);

    if (context->persistent.use_ping_cnd)
    {
        cnd_destroy(&context->persistent.ping_cnd);
        mtx_destroy(&context->persistent.ping_mtx);
    }

    {
        cnd_destroy(&context->persistent.pong_cnd);
        mtx_destroy(&context->persistent.pong_mtx);
    }

    free(context);
}

archi_status_t
archi_threads_execute(
        struct archi_threads_context *context,

        archi_threads_job_t job,
        archi_threads_callback_t callback,

        size_t batch_size,
        bool busy_wait)
{
    if ((context == NULL) || (job.function == NULL))
        return ARCHI_ERROR_MISUSE;

    if (job.num_tasks == 0)
        return 0;

    if (context->persistent.num_threads > 0)
    {
        // Check if threads are busy, and set the flag if not
        if (atomic_flag_test_and_set_explicit(&context->persistent.busy, memory_order_acquire))
            return 1;

        // Assign the job
        if (batch_size == 0) // automatic batch size
            batch_size = (job.num_tasks - 1) / context->persistent.num_threads + 1;

        context->current.job = job;
        context->current.callback = callback;
        context->current.batch_size = batch_size;
        context->current.use_pong_cnd = !busy_wait,

        // Initialize counters and flags
        context->current.done_tasks = 0;
        context->current.thread_counter = 0;

        bool ping_sense = context->persistent.ping_sense =
            !context->persistent.ping_sense;
        bool pong_sense = context->persistent.pong_sense =
            !context->persistent.pong_sense;

        // Wake slave threads
        atomic_store_explicit(&context->persistent.ping_flag, ping_sense, memory_order_release);

        if (context->persistent.use_ping_cnd)
        {
            {
#ifndef NDEBUG
                int res =
#endif
                    mtx_lock(&context->persistent.ping_mtx);
                assert(res == thrd_success);
            }
            cnd_broadcast(&context->persistent.ping_cnd);
            {
#ifndef NDEBUG
                int res =
#endif
                    mtx_unlock(&context->persistent.ping_mtx);
                assert(res == thrd_success);
            }
        }

        if (callback.function == NULL)
        {
            if (context->current.use_pong_cnd)
            {
#ifndef NDEBUG
                int res =
#endif
                    mtx_lock(&context->persistent.pong_mtx);
                assert(res == thrd_success);
            }

            // Wait until all tasks are done
            while (atomic_load_explicit(&context->persistent.pong_flag,
                        memory_order_acquire) != pong_sense)
            {
                if (context->current.use_pong_cnd)
                {
#ifndef NDEBUG
                    int res =
#endif
                        cnd_wait(&context->persistent.pong_cnd,
                                &context->persistent.pong_mtx);
                    assert(res == thrd_success);
                }
            }

            if (context->current.use_pong_cnd)
            {
#ifndef NDEBUG
                int res =
#endif
                    mtx_unlock(&context->persistent.pong_mtx);
                assert(res == thrd_success);
            }
        }
    }
    else
    {
        for (size_t task_idx = 0; task_idx < job.num_tasks; task_idx++)
            job.function(job.data, task_idx, 0);

        if (callback.function != NULL)
            callback.function(callback.data, job.num_tasks, 0);
    }

    return 0;
}

size_t
archi_threads_number(
        const struct archi_threads_context *context)
{
    return (context != NULL) ? context->persistent.num_threads : 0;
}

bool
archi_threads_busy_wait(
        const struct archi_threads_context *context)
{
    return (context != NULL) ? !context->persistent.use_ping_cnd : false;
}

