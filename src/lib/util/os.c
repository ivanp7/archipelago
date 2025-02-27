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

#include "archi/util/os/shm.fun.h"
#include "archi/util/os/lib.fun.h"
#include "archi/util/os/signal.fun.h"
#include "archi/util/print.fun.h"
#include "archi/util/value.typ.h"

#include <stdlib.h> // for malloc()

#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h> // for atomic_flag
#endif

#include <fcntl.h> // for open()
#include <unistd.h> // for close()
#include <sys/mman.h> // for mmap(), munmap()

#include <dlfcn.h> // for dlopen(), dlclose(), dlsym()

#include <signal.h> // for pthread_sigmask(), sigset_t, sigemptyset(), sigaddset(), siginfo_t, sigtimedwait()
#include <pthread.h> // for pthread_t, pthread_create(), phtread_join()
#include <time.h> // for timespec

int
archi_shm_open_file(
        const char *pathname,

        bool readable,
        bool writable)
{
    int flags = O_NONBLOCK;

    if (readable && writable)
        flags |= O_RDWR;
    else if (readable)
        flags |= O_RDONLY;
    else
        flags |= O_WRONLY;

    return open(pathname, flags);
}

bool
archi_shm_close(
        int fd)
{
    return close(fd) == 0;
}

archi_shm_header_t*
archi_shm_map(
        int fd,

        bool readable,
        bool writable,
        bool shared,
        int flags)
{
    int prot = (readable ? PROT_READ : 0) | (writable ? PROT_WRITE : 0);
    int all_flags = (shared ? MAP_SHARED_VALIDATE : MAP_PRIVATE) | flags;

    archi_shm_header_t *shm;
    archi_shm_header_t header;
    size_t size;

    // Map the memory the initial time to extract its header
    {
        shm = mmap(NULL, sizeof(*shm), prot, all_flags, fd, 0);
        if (shm == MAP_FAILED)
            return NULL;

        header = *shm;
        if (header.shmaddr > header.shmend)
            goto failure;

        size = (char*)header.shmend - (char*)header.shmaddr;
        if (size < sizeof(*shm))
            goto failure;
    }

    // Remap the memory of the correct size at the correct address
    {
        munmap(shm, sizeof(*shm));

        shm = mmap(header.shmaddr, size, prot, all_flags | MAP_FIXED_NOREPLACE, fd, 0);
        if (shm == MAP_FAILED)
            return NULL;

        if (shm != header.shmaddr)
            goto failure;
    }

    return shm;

failure:
    munmap(shm, sizeof(*shm));

    return NULL;
}

bool
archi_shm_unmap(
        archi_shm_header_t *shm)
{
    if (shm == NULL)
        return false;
    else if (shm->shmaddr > shm->shmend)
        return false;

    size_t size = (char*)shm->shmend - (char*)shm->shmaddr;

    return munmap(shm, size) == 0;
}

/*****************************************************************************/

void*
archi_library_load(
        const char *pathname,

        bool lazy,
        bool global)
{
    if (pathname == NULL)
        return NULL;

    return dlopen(pathname, (lazy ? RTLD_LAZY : RTLD_NOW) | (global ? RTLD_GLOBAL : RTLD_LOCAL));
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

    for (int i = 0; i < archi_signal_number_of_rt_signals(); i++)
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

    for (int i = 0; i < archi_signal_number_of_rt_signals(); i++)
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

    for (int i = 0; i < archi_signal_number_of_rt_signals(); i++)
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

        for (int i = 0; i < archi_signal_number_of_rt_signals(); i++)
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

