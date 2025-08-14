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
 * @brief Operations on signal sets.
 */

#include "archi/ipc_signal/api/signal.fun.h"

#include <stdlib.h> // for malloc()

#include <signal.h> // for SIGRTMIN, SIGRTMAX

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

