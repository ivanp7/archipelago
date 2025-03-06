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
 * @brief OS-specific utilities.
 */

#include "archi/util/os/file.fun.h"
#include "archi/util/os/library.fun.h"
#include "archi/util/os/signal.fun.h"
#include "archi/util/os/threads.fun.h"
#include "archi/util/os/queue.fun.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"
#include "archi/util/value.typ.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy(), memset()

#include <stdatomic.h> // for atomic_flag
#include <threads.h> // for thrd* functions and types

#include <fcntl.h> // for open()
#include <unistd.h> // for close(), sysconf()
#include <sys/mman.h> // for mmap(), munmap()
#include <sys/stat.h> // for fstat()

#include <dlfcn.h> // for dlopen(), dlclose(), dlsym()

#include <signal.h> // for pthread_sigmask(), sigset_t, sigemptyset(), sigaddset(), siginfo_t, sigtimedwait()
#include <pthread.h> // for pthread_t, pthread_create(), phtread_join()
#include <time.h> // for timespec

#include <stdint.h> // for uint_fast*_t
#include <limits.h> // for CHAR_BIT

#include <assert.h>

/*****************************************************************************/

size_t
archi_page_size(void)
{
    return sysconf(_SC_PAGE_SIZE);
}

int
archi_file_open(
        archi_file_open_config_t config)
{
    int flags = config.flags;

    if (config.readable && config.writable)
        flags |= O_RDWR;
    else if (config.readable)
        flags |= O_RDONLY;
    else
        flags |= O_WRONLY;

    if (config.nonblock)
        flags |= O_NONBLOCK;

    return open(config.pathname, flags);
}

bool
archi_file_close(
        int fd)
{
    return close(fd) == 0;
}

void*
archi_file_map(
        int fd,
        archi_file_map_config_t config,
        size_t *size)
{
    int prot = (config.readable ? PROT_READ : 0) | (config.writable ? PROT_WRITE : 0);
    int all_flags = (config.shared ? MAP_SHARED_VALIDATE : MAP_PRIVATE) | config.flags;

    if (!config.has_header)
    {
        if (config.size == 0)
        {
            struct stat statbuf;
            if (fstat(fd, &statbuf) != 0)
                return NULL;

            if (config.offset >= (size_t)statbuf.st_size)
                return NULL;

            config.size = statbuf.st_size - config.offset;
        }

        void *mm = mmap(NULL, config.size, prot, all_flags, fd, config.offset);
        if (mm == MAP_FAILED)
            return NULL;

        if (size != NULL)
            *size = config.size;

        return mm;
    }
    else
    {
        archi_mmap_header_t *mm;
        archi_mmap_header_t header;

        // Map the memory the initial time to extract its header
        mm = mmap(NULL, sizeof(header), prot, all_flags, fd, config.offset);
        if (mm == MAP_FAILED)
            return NULL;

        header = *mm;

        if (config.size == 0)
        {
            uintptr_t addr = (uintptr_t)header.addr;
            uintptr_t end  = (uintptr_t)header.end;

            if (addr > end)
            {
                munmap(mm, sizeof(header));
                return NULL;
            }

            config.size = end - addr;
        }

        if (config.size < sizeof(header))
        {
            munmap(mm, sizeof(header));
            return NULL;
        }

        // Remap the memory of the correct size at the correct address
        munmap(mm, sizeof(header));

        mm = mmap(header.addr, config.size, prot, all_flags | MAP_FIXED_NOREPLACE, fd, config.offset);
        if (mm == MAP_FAILED)
            return NULL;

        if (mm != header.addr)
        {
            munmap(mm, config.size);
            return NULL;
        }

        if (size != NULL)
            *size = config.size;

        return mm;
    }
}

bool
archi_file_unmap(
        void *mm,
        size_t size)
{
    if (mm == NULL)
        return false;

    return munmap(mm, size) == 0;
}

/*****************************************************************************/

void*
archi_library_load(
        archi_library_load_config_t config)
{
    if (config.pathname == NULL)
        return NULL;

    int flags = config.flags;

    if (config.lazy)
        flags |= RTLD_LAZY;
    else
        flags |= RTLD_NOW;

    if (config.global)
        flags |= RTLD_GLOBAL;
    else
        flags |= RTLD_LOCAL;

    return dlopen(config.pathname, flags);
}

void
archi_library_unload(
        void *handle)
{
    if (handle != NULL)
        dlclose(handle);
}

void*
archi_library_get_symbol(
        void *restrict handle,
        const char *restrict symbol)
{
    if ((handle == NULL) || (symbol == NULL))
        return NULL;

    return dlsym(handle, symbol);
}

void
archi_library_initialize_logging(
        void *handle)
{
    void *symbol;

    // Set the application start time
    symbol = archi_library_get_symbol(handle, "archi_log_set_start_time");
    if (symbol != NULL)
    {
        archi_log_set_start_time_func_t *fptr =
            (archi_log_set_start_time_func_t*)&symbol; // safe on POSIX systems

        struct timespec ts;
        archi_log_start_time(&ts);
        (*fptr)(&ts); // archi_log_set_start_time(&ts);
    }

    // Set logging verbosity
    symbol = archi_library_get_symbol(handle, "archi_log_set_verbosity");
    if (symbol != NULL)
    {
        archi_log_set_verbosity_func_t *fptr =
            (archi_log_set_verbosity_func_t*)&symbol; // safe on POSIX systems

        int verbosity_level = archi_log_verbosity();
        (*fptr)(verbosity_level); // archi_log_set_verbosity(verbosity_level);
    }
}

/*****************************************************************************/

size_t
archi_signal_number_of_rt_signals(void)
{
    return (SIGRTMAX - SIGRTMIN + 1);
}

void
archi_signal_watch_set_join(
        archi_signal_watch_set_t *out,
        const archi_signal_watch_set_t *in)
{
    if ((out == NULL) || (in == NULL))
        return;

#define JOIN_SIGNAL(signal) do { \
        out->f_##signal = out->f_##signal || in->f_##signal; \
    } while (0)

    JOIN_SIGNAL(SIGINT);
    JOIN_SIGNAL(SIGQUIT);
    JOIN_SIGNAL(SIGTERM);

    JOIN_SIGNAL(SIGCHLD);
    JOIN_SIGNAL(SIGCONT);
    JOIN_SIGNAL(SIGTSTP);
    JOIN_SIGNAL(SIGXCPU);
    JOIN_SIGNAL(SIGXFSZ);

    JOIN_SIGNAL(SIGPIPE);
    JOIN_SIGNAL(SIGPOLL);
    JOIN_SIGNAL(SIGURG);

    JOIN_SIGNAL(SIGALRM);
    JOIN_SIGNAL(SIGVTALRM);
    JOIN_SIGNAL(SIGPROF);

    JOIN_SIGNAL(SIGHUP);
    JOIN_SIGNAL(SIGTTIN);
    JOIN_SIGNAL(SIGTTOU);
    JOIN_SIGNAL(SIGWINCH);

    JOIN_SIGNAL(SIGUSR1);
    JOIN_SIGNAL(SIGUSR2);

#undef JOIN_SIGNAL

    for (size_t i = 0; i < archi_signal_number_of_rt_signals(); i++)
        out->f_SIGRTMIN[i] = out->f_SIGRTMIN[i] || in->f_SIGRTMIN[i];
}

archi_signal_watch_set_t*
archi_signal_watch_set_alloc(void)
{
    size_t size = ARCHI_SIGNAL_WATCH_SET_SIZEOF;
    archi_signal_watch_set_t *signals = malloc(size);
    if (signals == NULL)
        return NULL;

#define INIT_SIGNAL(signal) do { \
        signals->f_##signal = false; \
    } while (0)

    INIT_SIGNAL(SIGINT);
    INIT_SIGNAL(SIGQUIT);
    INIT_SIGNAL(SIGTERM);

    INIT_SIGNAL(SIGCHLD);
    INIT_SIGNAL(SIGCONT);
    INIT_SIGNAL(SIGTSTP);
    INIT_SIGNAL(SIGXCPU);
    INIT_SIGNAL(SIGXFSZ);

    INIT_SIGNAL(SIGPIPE);
    INIT_SIGNAL(SIGPOLL);
    INIT_SIGNAL(SIGURG);

    INIT_SIGNAL(SIGALRM);
    INIT_SIGNAL(SIGVTALRM);
    INIT_SIGNAL(SIGPROF);

    INIT_SIGNAL(SIGHUP);
    INIT_SIGNAL(SIGTTIN);
    INIT_SIGNAL(SIGTTOU);
    INIT_SIGNAL(SIGWINCH);

    INIT_SIGNAL(SIGUSR1);
    INIT_SIGNAL(SIGUSR2);

#undef INIT_SIGNAL

    for (size_t i = 0; i < archi_signal_number_of_rt_signals(); i++)
        signals->f_SIGRTMIN[i] = false;

    return signals;
}

archi_signal_flags_t*
archi_signal_flags_alloc(void)
{
    size_t size = ARCHI_SIGNAL_FLAGS_SIZEOF;
    archi_signal_flags_t *signals = malloc(size);
    if (signals == NULL)
        return NULL;

#define INIT_SIGNAL(signal) do { \
        ARCHI_SIGNAL_INIT_FLAG(signals->f_##signal); \
    } while (0)

    INIT_SIGNAL(SIGINT);
    INIT_SIGNAL(SIGQUIT);
    INIT_SIGNAL(SIGTERM);

    INIT_SIGNAL(SIGCHLD);
    INIT_SIGNAL(SIGCONT);
    INIT_SIGNAL(SIGTSTP);
    INIT_SIGNAL(SIGXCPU);
    INIT_SIGNAL(SIGXFSZ);

    INIT_SIGNAL(SIGPIPE);
    INIT_SIGNAL(SIGPOLL);
    INIT_SIGNAL(SIGURG);

    INIT_SIGNAL(SIGALRM);
    INIT_SIGNAL(SIGVTALRM);
    INIT_SIGNAL(SIGPROF);

    INIT_SIGNAL(SIGHUP);
    INIT_SIGNAL(SIGTTIN);
    INIT_SIGNAL(SIGTTOU);
    INIT_SIGNAL(SIGWINCH);

    INIT_SIGNAL(SIGUSR1);
    INIT_SIGNAL(SIGUSR2);

#undef INIT_SIGNAL

    for (size_t i = 0; i < archi_signal_number_of_rt_signals(); i++)
        ARCHI_SIGNAL_INIT_FLAG(signals->f_SIGRTMIN[i]);

    return signals;
}

struct archi_signal_management_context
{
    archi_signal_flags_t *flags;

    archi_signal_handler_t signal_handler;
#ifndef __STDC_NO_ATOMICS__
    atomic_flag spinlock;
#endif

    pthread_t thread;
    sigset_t set;

    bool thread_running;
    archi_signal_flag_t terminate;
};

static
void*
archi_signal_management_thread(
        void *arg)
{
    struct archi_signal_management_context *context = arg;

    siginfo_t siginfo;
    struct timespec delay = {.tv_sec = 0, .tv_nsec = 1000000}; // 1 ms

    while (!ARCHI_SIGNAL_IS_FLAG_SET(context->terminate))
    {
        int signal = sigtimedwait(&context->set, &siginfo, &delay);
        if (signal <= 0)
            continue;

        archi_signal_handler_t signal_handler = archi_signal_management_handler(context);

        if ((signal_handler.function != NULL) &&
                !signal_handler.function(signal, &siginfo, context->flags, signal_handler.data))
            continue;

        switch (signal)
        {
#define CASE_SIGNAL(signal)                                         \
            case signal:                                            \
                ARCHI_SIGNAL_SET_FLAG(context->flags->f_##signal);  \
                break

            CASE_SIGNAL(SIGINT);
            CASE_SIGNAL(SIGQUIT);
            CASE_SIGNAL(SIGTERM);

            CASE_SIGNAL(SIGCHLD);
            CASE_SIGNAL(SIGCONT);
            CASE_SIGNAL(SIGTSTP);
            CASE_SIGNAL(SIGXCPU);
            CASE_SIGNAL(SIGXFSZ);

            CASE_SIGNAL(SIGPIPE);
            CASE_SIGNAL(SIGPOLL);
            CASE_SIGNAL(SIGURG);

            CASE_SIGNAL(SIGALRM);
            CASE_SIGNAL(SIGVTALRM);
            CASE_SIGNAL(SIGPROF);

            CASE_SIGNAL(SIGHUP);
            CASE_SIGNAL(SIGTTIN);
            CASE_SIGNAL(SIGTTOU);
            CASE_SIGNAL(SIGWINCH);

            CASE_SIGNAL(SIGUSR1);
            CASE_SIGNAL(SIGUSR2);

#undef CASE_SIGNAL

            default:
                if ((signal >= SIGRTMIN) && (signal <= SIGRTMAX))
                    ARCHI_SIGNAL_SET_FLAG(context->flags->f_SIGRTMIN[signal - SIGRTMIN]);
        }
    }

    return NULL;
}

struct archi_signal_management_context*
archi_signal_management_start(
        const archi_signal_watch_set_t *signals,
        archi_signal_handler_t signal_handler)
{
    if (signals == NULL)
        return NULL;

    struct archi_signal_management_context *context = malloc(sizeof(*context));
    if (context == NULL)
        return NULL;

    *context = (struct archi_signal_management_context){
        .signal_handler = signal_handler,
        .spinlock = ATOMIC_FLAG_INIT,
    };

    context->flags = archi_signal_flags_alloc();
    if (context->flags == NULL)
        goto failure;

    sigemptyset(&context->set);

    ARCHI_SIGNAL_INIT_FLAG(context->terminate);

    {
#define ADD_SIGNAL(signal) do {     \
        if (signals->f_##signal) {  \
            sigaddset(&context->set, signal); } } while (0)

        ADD_SIGNAL(SIGINT);
        ADD_SIGNAL(SIGQUIT);
        ADD_SIGNAL(SIGTERM);

        ADD_SIGNAL(SIGCHLD);
        ADD_SIGNAL(SIGCONT);
        ADD_SIGNAL(SIGTSTP);
        ADD_SIGNAL(SIGXCPU);
        ADD_SIGNAL(SIGXFSZ);

        ADD_SIGNAL(SIGPIPE);
        ADD_SIGNAL(SIGPOLL);
        ADD_SIGNAL(SIGURG);

        ADD_SIGNAL(SIGALRM);
        ADD_SIGNAL(SIGVTALRM);
        ADD_SIGNAL(SIGPROF);

        ADD_SIGNAL(SIGHUP);
        ADD_SIGNAL(SIGTTIN);
        ADD_SIGNAL(SIGTTOU);
        ADD_SIGNAL(SIGWINCH);

        ADD_SIGNAL(SIGUSR1);
        ADD_SIGNAL(SIGUSR2);

#undef ADD_SIGNAL

        for (size_t i = 0; i < archi_signal_number_of_rt_signals(); i++)
        {
            if (signals->f_SIGRTMIN[i])
                sigaddset(&context->set, SIGRTMIN+i);
        }
    }

    if (pthread_sigmask(SIG_BLOCK, &context->set, (sigset_t*)NULL) != 0)
        goto failure;

    if (pthread_create(&context->thread, NULL, archi_signal_management_thread, context) != 0)
    {
        pthread_sigmask(SIG_UNBLOCK, &context->set, (sigset_t*)NULL);
        goto failure;
    }

    context->thread_running = true;

    return context;

failure:
    archi_signal_management_stop(context);
    return NULL;
}

void
archi_signal_management_stop(
        struct archi_signal_management_context *context)
{
    if (context == NULL)
        return;

    if (context->thread_running)
    {
        ARCHI_SIGNAL_SET_FLAG(context->terminate);
        pthread_join(context->thread, (void**)NULL);
        pthread_sigmask(SIG_UNBLOCK, &context->set, (sigset_t*)NULL);
    }

    free(context->flags);
    free(context);
}

archi_signal_flags_t*
archi_signal_management_flags(
        struct archi_signal_management_context *context)
{
    if (context == NULL)
        return NULL;

    return context->flags;
}

archi_signal_handler_t
archi_signal_management_handler(
        struct archi_signal_management_context *context)
{
    if (context == NULL)
        return (archi_signal_handler_t){0};

    archi_signal_handler_t signal_handler;
    {
#ifndef __STDC_NO_ATOMICS__
        while (atomic_flag_test_and_set_explicit(&context->spinlock, memory_order_acquire)); // lock
#endif
        signal_handler = context->signal_handler;
#ifndef __STDC_NO_ATOMICS__
        atomic_flag_clear_explicit(&context->spinlock, memory_order_release); // unlock
#endif
    }

    return signal_handler;
}

void
archi_signal_management_set_handler(
        struct archi_signal_management_context *context,

        archi_signal_handler_t signal_handler)
{
    if (context == NULL)
        return;

#ifndef __STDC_NO_ATOMICS__
    while (atomic_flag_test_and_set_explicit(&context->spinlock, memory_order_acquire)); // lock
#endif
    context->signal_handler = signal_handler;
#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear_explicit(&context->spinlock, memory_order_release); // unlock
#endif
}

/*****************************************************************************/

struct archi_thread_group_context {
    struct {
        thrd_t *threads;
        archi_thread_group_config_t config;

        atomic_flag busy;

        atomic_bool ping_flag;
        atomic_bool pong_flag;
        bool ping_sense;
        bool pong_sense;

        cnd_t ping_cnd;
        mtx_t ping_mtx;

        cnd_t pong_cnd;
        mtx_t pong_mtx;
    } persistent;

    struct {
        archi_thread_group_job_t job;
        archi_thread_group_callback_t callback;
        archi_thread_group_exec_config_t config;

        atomic_uint done_tasks;
        atomic_ushort thread_counter;
    } current;

    bool terminate;
};

struct archi_thread_arg {
    struct archi_thread_group_context *context;
    size_t thread_idx;
};

static
int
archi_thread(
        void *arg)
{
    struct archi_thread_group_context *context;
    size_t thread_idx;
    {
        struct archi_thread_arg *thread_arg = arg;
        assert(thread_arg != NULL);

        context = thread_arg->context;
        thread_idx = thread_arg->thread_idx;

        // Free a temporary object
        free(thread_arg);
    }

    size_t thread_counter_last = context->persistent.config.num_threads - 1;
    bool use_ping_cnd = !context->persistent.config.busy_wait;

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
        archi_thread_group_job_t job = context->current.job;
        size_t batch_size = context->current.config.batch_size;

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
            else if (!context->current.config.busy_wait)
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

struct archi_thread_group_context*
archi_thread_group_start(
        archi_thread_group_config_t config,

        archi_status_t *code)
{
    archi_status_t status = 0;

    size_t thread_idx = 0;

    // Initialize threads context
    struct archi_thread_group_context *context = malloc(sizeof(*context));
    if (context == NULL)
    {
        status = ARCHI_ERROR_ALLOC;
        goto failure;
    }

    *context = (struct archi_thread_group_context){.persistent = {
        .config = config,
        .busy = ATOMIC_FLAG_INIT,
    }};

    // Create mutexes and condition variables
    if (!config.busy_wait)
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
        context->persistent.config.num_threads = thread_idx; // number of created threads
        archi_thread_group_stop(context);
    }

    if (code != NULL)
        *code = status;

    return NULL;
}

void
archi_thread_group_stop(
        struct archi_thread_group_context *context)
{
    if (context == NULL)
        return;

    // Wake threads
    context->terminate = true;

    atomic_store_explicit(&context->persistent.ping_flag,
            !context->persistent.ping_sense, memory_order_release);

    bool use_ping_cnd = !context->persistent.config.busy_wait;

    if (use_ping_cnd)
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
    for (size_t i = 0; i < context->persistent.config.num_threads; i++)
        thrd_join(context->persistent.threads[i], (int*)NULL);

    // Destroy mutexes, condition variables, and free memory
    free(context->persistent.threads);

    if (use_ping_cnd)
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
archi_thread_group_execute(
        struct archi_thread_group_context *context,

        archi_thread_group_job_t job,
        archi_thread_group_callback_t callback,
        archi_thread_group_exec_config_t config)
{
    if ((context == NULL) || (job.function == NULL))
        return ARCHI_ERROR_MISUSE;

    if (job.num_tasks == 0)
        return 0;

    if (context->persistent.config.num_threads > 0)
    {
        // Check if threads are busy, and set the flag if not
        if (atomic_flag_test_and_set_explicit(&context->persistent.busy, memory_order_acquire))
            return 1;

        // Assign the job
        if (config.batch_size == 0) // automatic batch size
            config.batch_size = (job.num_tasks - 1) / context->persistent.config.num_threads + 1;

        context->current.job = job;
        context->current.callback = callback;
        context->current.config = config;

        // Initialize counters and flags
        context->current.done_tasks = 0;
        context->current.thread_counter = 0;

        bool ping_sense = context->persistent.ping_sense =
            !context->persistent.ping_sense;
        bool pong_sense = context->persistent.pong_sense =
            !context->persistent.pong_sense;

        // Wake slave threads
        atomic_store_explicit(&context->persistent.ping_flag, ping_sense, memory_order_release);

        if (!context->persistent.config.busy_wait)
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
            if (!config.busy_wait)
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
                if (!config.busy_wait)
                {
#ifndef NDEBUG
                    int res =
#endif
                        cnd_wait(&context->persistent.pong_cnd,
                                &context->persistent.pong_mtx);
                    assert(res == thrd_success);
                }
            }

            if (!config.busy_wait)
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

archi_thread_group_config_t
archi_thread_group_config(
        const struct archi_thread_group_context *context)
{
    if (context == NULL)
        return (archi_thread_group_config_t){0};

    return context->persistent.config;
}

/*****************************************************************************/

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

