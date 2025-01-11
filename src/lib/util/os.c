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
 * @brief OS-specific utilities.
 */

#include "archi/util/os/shm.fun.h"
#include "archi/util/os/lib.fun.h"
#include "archi/util/os/signal.fun.h"

#include <stdlib.h> // for malloc()

#include <sys/shm.h> // for ftok()
#include <sys/ipc.h> // for shmget(), shmat(), shmdt()

#include <dlfcn.h> // for dlopen(), dlclose(), dlsym()

#include <signal.h> // for pthread_sigmask(), sigset_t, sigemptyset(), sigaddset(), siginfo_t, sigtimedwait()
#include <pthread.h> // for pthread_t, pthread_create(), phtread_join()
#include <time.h> // for timespec

void**
archi_shared_memory_create(
        const char *pathname,
        int proj_id,

        size_t size)
{
    if (pathname == NULL)
        return NULL;
    else if (size <= sizeof(void*))
        return NULL;

    key_t key = ftok(pathname, proj_id);
    if (key == -1)
        return NULL;

    int shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0640);
    if (shmid == -1)
        return NULL;

    void **shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void*)-1)
        goto failure;

    shmaddr[0] = shmaddr;

    return shmaddr;

failure:
    archi_shared_memory_destroy(pathname, proj_id);
    return NULL;
}

bool
archi_shared_memory_destroy(
        const char *pathname,
        int proj_id)
{
    if (pathname == NULL)
        return false;

    key_t key = ftok(pathname, proj_id);
    if (key == -1)
        return false;

    int shmid = shmget(key, 0, 0);
    if (shmid == -1)
        return false;

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        return false;

    return true;
}

void**
archi_shared_memory_attach(
        const char *pathname,
        const int proj_id,

        bool writable)
{
    if (pathname == NULL)
        return NULL;

    key_t key = ftok(pathname, proj_id);
    if (key == -1)
        return NULL;

    int shmid = shmget(key, 0, 0);
    if (shmid == -1)
        return NULL;

    int shmflg = writable ? 0 : SHM_RDONLY;

    void **current_shmaddr = shmat(shmid, NULL, shmflg);
    if (current_shmaddr == (void*)-1)
        return NULL;

    void *shmaddr = current_shmaddr[0];

    if (shmaddr != current_shmaddr)
    {
        if (shmdt(current_shmaddr) == -1)
            return NULL;

        current_shmaddr = shmat(shmid, shmaddr, SHM_RND | shmflg);
        if (current_shmaddr == (void*)-1)
            return NULL;

        if (current_shmaddr != shmaddr)
        {
            shmdt(current_shmaddr);
            return NULL;
        }
    }

    return shmaddr;
}

bool
archi_shared_memory_detach(
        const void *shmaddr)
{
    if (shmaddr == NULL)
        return true;

    return shmdt(shmaddr) == 0;
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

/*****************************************************************************/

size_t
archi_signal_number_of_rt_signals(void)
{
    return (SIGRTMAX - SIGRTMIN + 1);
}

archi_signal_watch_set_t*
archi_signal_watch_set_alloc(void)
{
    archi_signal_watch_set_t *signals = malloc(ARCHI_SIGNAL_WATCH_SET_SIZEOF);
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

    for (int signal = SIGRTMIN; signal <= SIGRTMAX; signal++)
        signals->f_SIGRTMIN[signal - SIGRTMIN] = false;

    return signals;
}

archi_signal_flags_t*
archi_signal_flags_alloc(void)
{
    archi_signal_flags_t *signals = malloc(ARCHI_SIGNAL_FLAGS_SIZEOF);
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

    for (int signal = SIGRTMIN; signal <= SIGRTMAX; signal++)
        ARCHI_SIGNAL_INIT_FLAG(signals->f_SIGRTMIN[signal - SIGRTMIN]);

    return signals;
}

struct archi_signal_management_context
{
    archi_signal_flags_t *flags;
    archi_signal_handler_t signal_handler;

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

        bool set_flag;
        if (context->signal_handler.function != NULL)
            set_flag = context->signal_handler.function(signal, &siginfo,
                    context->flags, context->signal_handler.data);
        else
            set_flag = true;

        if (!set_flag)
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
archi_signal_management_thread_start(
        const archi_signal_watch_set_t *signals,
        archi_signal_handler_t signal_handler)
{
    if (signals == NULL)
        return NULL;

    struct archi_signal_management_context *context = malloc(sizeof(*context));
    if (context == NULL)
        return NULL;

    *context = (struct archi_signal_management_context){0};

    context->flags = archi_signal_flags_alloc();
    if (context->flags == NULL)
        goto failure;

    context->signal_handler = signal_handler;

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

        for (int signal = SIGRTMIN; signal <= SIGRTMAX; signal++)
        {
            if (signals->f_SIGRTMIN[signal - SIGRTMIN])
                sigaddset(&context->set, signal);
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
    archi_signal_management_thread_stop(context);
    return NULL;
}

void
archi_signal_management_thread_stop(
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

void
archi_signal_management_thread_get_properties(
        struct archi_signal_management_context *context,

        archi_signal_flags_t **flags,
        archi_signal_handler_t *signal_handler)
{
    if (context == NULL)
        return;

    if (flags != NULL)
        *flags = context->flags;

    if (signal_handler != NULL)
        *signal_handler = context->signal_handler;
}

