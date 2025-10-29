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

#include "archi_exe/signal_management.fun.h"
#include "archi/signal/api/signal.fun.h"
#include "archi/signal/api/management.fun.h"
#include "../archi/signal/api/management.typ.h" // for struct archi_signal_management_context_base

#include <stdlib.h> // for malloc(), free()
#include <threads.h> // for mtx_*
#include <signal.h> // for pthread_sigmask(), sigset_t, sigemptyset(), sigaddset(), siginfo_t, sigtimedwait()
#include <pthread.h> // for pthread_t, pthread_create(), phtread_join()
#include <time.h> // for timespec

static
struct archi_signal_management_context
{
    struct archi_signal_management_context_base base; // Flags and signal handler.

    pthread_t thread;
    sigset_t set;

    bool thread_running;
    archi_signal_flag_t terminate;
} archi_signal_management;

ARCHI_GLOBAL_GET_FUNC(archi_exe_signal_management_global_context)
{
    return &archi_signal_management;
}

static
void*
archi_exe_signal_management_thread(
        void *arg)
{
    (void) arg;

    siginfo_t siginfo;
    struct timespec interval = {.tv_sec = 0, .tv_nsec = 1000000}; // 1 ms

    while (!ARCHI_SIGNAL_FLAG_IS_SET(archi_signal_management.terminate))
    {
        // Wait for a signal up to the specified time interval
        int signal = sigtimedwait(&archi_signal_management.set, &siginfo, &interval);
        if (signal <= 0) // no signal received during the time interval
            continue;

        bool set_flag = true;
        {
            // Thread-safely extract the current signal handler
            archi_signal_handler_t signal_handler = archi_signal_management_handler();

            // Don't set the signal flag only if the signal handler function returns false
            if ((signal_handler.function != NULL) &&
                    !signal_handler.function(signal, &siginfo,
                        archi_signal_management.base.flags, signal_handler.data))
                set_flag = false;
        }

        // Set the signal flag if needed
        if (set_flag)
        {
            switch (signal)
            {
#define CASE_SIGNAL(signal)                                                                 \
                case signal:                                                                \
                    ARCHI_SIGNAL_FLAG_SET(archi_signal_management.base.flags->f_##signal);  \
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
                        ARCHI_SIGNAL_FLAG_SET(archi_signal_management.base.flags->f_SIGRTMIN[signal - SIGRTMIN]);
            }
        }
    }

    return NULL;
}

void
archi_exe_signal_management_start(
        archi_signal_watch_set_t *signals,
        archi_signal_handler_t signal_handler,
        ARCHI_ERROR_PARAMETER_DECL)
{
    if (archi_signal_management.thread_running)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "signal management thread is already running");
        return;
    }
    else if (signals == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "signal watch set is NULL");
        return;
    }

    // Allocate signal flags
    archi_signal_management.base.flags = archi_signal_flags_alloc();
    if (archi_signal_management.base.flags == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate signal flags structure");
        goto failure_alloc_flags;
    }

    // Allocate signal handler protection mutex
    archi_signal_management.base.signal_handler_lock = malloc(
            sizeof(*archi_signal_management.base.signal_handler_lock));
    if (archi_signal_management.base.signal_handler_lock == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate signal handler mutex");
        goto failure_alloc_lock;
    }

    // Initialize signal handler protection mutex
    {
        int ret = mtx_init(archi_signal_management.base.signal_handler_lock, mtx_plain);
        if (ret != thrd_success)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize signal handler lock");
            goto failure_init_lock;
        }
    }

    // Set initial signal handler
    archi_signal_management.base.signal_handler = signal_handler;

    // Initialize the thread termination flag
    ARCHI_SIGNAL_FLAG_INIT(archi_signal_management.terminate);

    // Initialize set of signals to block
    sigemptyset(&archi_signal_management.set);

    {
#define ADD_SIGNAL(signal) do {     \
        if (signals->f_##signal) {  \
            sigaddset(&archi_signal_management.set, signal); } } while (0)

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
                sigaddset(&archi_signal_management.set, SIGRTMIN+i);
        }
    }

    // Block the specified signals
    if (pthread_sigmask(SIG_BLOCK, &archi_signal_management.set, (sigset_t*)NULL) != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "can't block signals");
        goto failure_sigmask;
    }

    // Create the signal management thread
    if (pthread_create(&archi_signal_management.thread, NULL,
                archi_exe_signal_management_thread, NULL) != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't start signal management thread");
        goto failure_thread;
    }

    archi_signal_management.thread_running = true;

    ARCHI_ERROR_RESET();
    return;

    // Undo the progress in case of failure
failure_thread:
    pthread_sigmask(SIG_UNBLOCK, &archi_signal_management.set, (sigset_t*)NULL);

failure_sigmask:
    mtx_destroy(archi_signal_management.base.signal_handler_lock);

failure_init_lock:
    free(archi_signal_management.base.signal_handler_lock);

failure_alloc_lock:
    free(archi_signal_management.base.flags);

failure_alloc_flags:
    archi_signal_management = (struct archi_signal_management_context){0};
}

void
archi_exe_signal_management_stop(void)
{
    if (!archi_signal_management.thread_running)
        return;

    // Set the thread termination flag and wait for it to terminate
    ARCHI_SIGNAL_FLAG_SET(archi_signal_management.terminate);
    pthread_join(archi_signal_management.thread, (void**)NULL);

    // Unlock the specified signals
    pthread_sigmask(SIG_UNBLOCK, &archi_signal_management.set, (sigset_t*)NULL);

    // Destroy and deallocate the signal handler protection mutex
    mtx_destroy(archi_signal_management.base.signal_handler_lock);
    free(archi_signal_management.base.signal_handler_lock);

    // Deallocate signal flags
    free(archi_signal_management.base.flags);

    // Clean the context state
    archi_signal_management = (struct archi_signal_management_context){0};
}

