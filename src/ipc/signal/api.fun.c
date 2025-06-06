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
 * @brief Signal management operations.
 */

#include "archi/ipc/signal/api.fun.h"

#include <stdlib.h> // for malloc(), free()

#include <signal.h> // for pthread_sigmask(), sigset_t, sigemptyset(), sigaddset(), siginfo_t, sigtimedwait()
#include <pthread.h> // for pthread_t, pthread_create(), phtread_join()
#include <time.h> // for timespec

size_t
archi_signal_number_of_rt_signals(void)
{
    return (SIGRTMAX - SIGRTMIN + 1);
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

bool
archi_signal_watch_set_not_empty(
        const archi_signal_watch_set_t *signals)
{
    if (signals == NULL)
        return false;

#define TEST_SIGNAL(signal) do { \
        if (signals->f_##signal) \
            return true;         \
    } while (0)

    TEST_SIGNAL(SIGINT);
    TEST_SIGNAL(SIGQUIT);
    TEST_SIGNAL(SIGTERM);

    TEST_SIGNAL(SIGCHLD);
    TEST_SIGNAL(SIGCONT);
    TEST_SIGNAL(SIGTSTP);
    TEST_SIGNAL(SIGXCPU);
    TEST_SIGNAL(SIGXFSZ);

    TEST_SIGNAL(SIGPIPE);
    TEST_SIGNAL(SIGPOLL);
    TEST_SIGNAL(SIGURG);

    TEST_SIGNAL(SIGALRM);
    TEST_SIGNAL(SIGVTALRM);
    TEST_SIGNAL(SIGPROF);

    TEST_SIGNAL(SIGHUP);
    TEST_SIGNAL(SIGTTIN);
    TEST_SIGNAL(SIGTTOU);
    TEST_SIGNAL(SIGWINCH);

    TEST_SIGNAL(SIGUSR1);
    TEST_SIGNAL(SIGUSR2);

#undef TEST_SIGNAL

    for (size_t i = 0; i < archi_signal_number_of_rt_signals(); i++)
    {
        if (signals->f_SIGRTMIN[i])
            return true;
    }

    return false;
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
    atomic_flag spinlock;

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
    archi_signal_management_context_t context = arg;

    siginfo_t siginfo;
    struct timespec delay = {.tv_sec = 0, .tv_nsec = 1000000}; // 1 ms

    while (!ARCHI_SIGNAL_IS_FLAG_SET(context->terminate))
    {
        int signal = sigtimedwait(&context->set, &siginfo, &delay);
        if (signal <= 0)
            continue;

        archi_signal_handler_t signal_handler = archi_signal_management_handler(context);

        if ((signal_handler.function == NULL) ||
                signal_handler.function(signal, &siginfo, context->flags, signal_handler.data))
        {
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
    }

    return NULL;
}

archi_signal_management_context_t
archi_signal_management_start(
        archi_signal_management_start_params_t params,
        archi_status_t *code)
{
    if (params.signals == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    archi_signal_management_context_t context = malloc(sizeof(*context));
    if (context == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    *context = (struct archi_signal_management_context){
        .signal_handler = params.signal_handler,
        .spinlock = ATOMIC_FLAG_INIT,
    };

    context->flags = archi_signal_flags_alloc();
    if (context->flags == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto failure;
    }

    sigemptyset(&context->set);

    ARCHI_SIGNAL_INIT_FLAG(context->terminate);

    {
#define ADD_SIGNAL(signal) do {     \
        if (params.signals->f_##signal) {  \
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
            if (params.signals->f_SIGRTMIN[i])
                sigaddset(&context->set, SIGRTMIN+i);
        }
    }

    if (pthread_sigmask(SIG_BLOCK, &context->set, (sigset_t*)NULL) != 0)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        goto failure;
    }

    if (pthread_create(&context->thread, NULL, archi_signal_management_thread, context) != 0)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        pthread_sigmask(SIG_UNBLOCK, &context->set, (sigset_t*)NULL);
        goto failure;
    }

    context->thread_running = true;

    if (code != NULL)
        *code = 0;

    return context;

failure:
    archi_signal_management_stop(context);
    return NULL;
}

void
archi_signal_management_stop(
        archi_signal_management_context_t context)
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
        archi_signal_management_context_t context)
{
    if (context == NULL)
        return NULL;

    return context->flags;
}

archi_signal_handler_t
archi_signal_management_handler(
        archi_signal_management_context_t context)
{
    if (context == NULL)
        return (archi_signal_handler_t){0};

    archi_signal_handler_t signal_handler;
    {
        while (atomic_flag_test_and_set_explicit(&context->spinlock, memory_order_acquire)); // lock

        signal_handler = context->signal_handler;

        atomic_flag_clear_explicit(&context->spinlock, memory_order_release); // unlock
    }

    return signal_handler;
}

void
archi_signal_management_set_handler(
        archi_signal_management_context_t context,
        archi_signal_handler_t signal_handler)
{
    if (context == NULL)
        return;

    {
        while (atomic_flag_test_and_set_explicit(&context->spinlock, memory_order_acquire)); // lock

        context->signal_handler = signal_handler;

        atomic_flag_clear_explicit(&context->spinlock, memory_order_release); // unlock
    }
}

