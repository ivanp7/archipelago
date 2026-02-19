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
 * @brief Operations on signal sets.
 */

#include "archi/signal/api/signal.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for malloc()

#include <signal.h> // for SIGRTMIN, SIGRTMAX


int
archi_signal_number_realtime(void)
{
    if (SIGRTMIN > SIGRTMAX) // real-time signals not supported
        return 0;

    return SIGRTMAX - SIGRTMIN + 1;
}

const char*
archi_signal_name[ARCHI_SIGNAL_NUMBER + 1] = {
    "SIGINT",
    "SIGQUIT",
    "SIGTERM",

    "SIGCHLD",
    "SIGCONT",
    "SIGTSTP",
    "SIGXCPU",
    "SIGXFSZ",

    "SIGPIPE",
    "SIGPOLL",
    "SIGURG",

    "SIGALRM",
    "SIGVTALRM",
    "SIGPROF",

    "SIGHUP",
    "SIGTTIN",
    "SIGTTOU",
    "SIGWINCH",

    "SIGUSR1",
    "SIGUSR2",

    "SIGRTMIN",
};

const char*
archi_signal_name__SIGRTMAX = "SIGRTMAX";

/*****************************************************************************/

archi_signal_set_t
archi_signal_set_alloc(void)
{
    archi_signal_set_t set = malloc(sizeof(archi_signal_set_mask_t) * ARCHI_SIGNAL_SET_NUM_MASKS);
    if (set == NULL)
        return NULL;

    for (size_t i = 0; i < ARCHI_SIGNAL_SET_NUM_MASKS; i++)
        set[i] = 0;

    return set;
}

static
archi_signal_set_mask_t
archi_signal_set_tail_mask(void)
{
    int tail_bits = (ARCHI_SIGNAL_NUMBER + ARCHI_SIGNAL_NUMBER_REALTIME) % ARCHI_SIGNAL_SET_MASK_NUM_BITS;
    if (tail_bits == 0)
        tail_bits = ARCHI_SIGNAL_SET_MASK_NUM_BITS;

    archi_signal_set_mask_t tail_mask = -1;
    tail_mask >>= ARCHI_SIGNAL_SET_MASK_NUM_BITS - tail_bits;

    return tail_mask;
}

bool
archi_signal_set_is_empty(
        archi_signal_set_const_t set)
{
    if (set == NULL)
        return true;

    for (size_t i = 0; i < ARCHI_SIGNAL_SET_NUM_MASKS - 1; i++)
        if (set[i] != 0)
            return false;

     // Check used bits only, ignoring possible garbage bits
    if ((set[ARCHI_SIGNAL_SET_NUM_MASKS - 1] & archi_signal_set_tail_mask()) != 0)
        return false;

    return true;
}

void
archi_signal_set_clear(
        archi_signal_set_t set)
{
    if (set == NULL)
        return;

    for (size_t i = 0; i < ARCHI_SIGNAL_SET_NUM_MASKS; i++)
        set[i] = 0;
}

void
archi_signal_set_invert(
        archi_signal_set_t set)
{
    if (set == NULL)
        return;

    for (size_t i = 0; i < ARCHI_SIGNAL_SET_NUM_MASKS; i++)
        set[i] = ~set[i];

     // Unset unused bits
    set[ARCHI_SIGNAL_SET_NUM_MASKS - 1] &= archi_signal_set_tail_mask();
}

void
archi_signal_set_assign(
        archi_signal_set_t out,
        archi_signal_set_const_t in)
{
    if ((out == NULL) || (in == NULL))
        return;

    for (size_t i = 0; i < ARCHI_SIGNAL_SET_NUM_MASKS; i++)
        out[i] = in[i];

     // Unset unused bits
    out[ARCHI_SIGNAL_SET_NUM_MASKS - 1] &= archi_signal_set_tail_mask();
}

void
archi_signal_set_join(
        archi_signal_set_t out,
        archi_signal_set_const_t in)
{
    if ((out == NULL) || (in == NULL))
        return;

    for (size_t i = 0; i < ARCHI_SIGNAL_SET_NUM_MASKS; i++)
        out[i] |= in[i];

     // Unset unused bits
    out[ARCHI_SIGNAL_SET_NUM_MASKS - 1] &= archi_signal_set_tail_mask();
}

void
archi_signal_set_intersect(
        archi_signal_set_t out,
        archi_signal_set_const_t in)
{
    if ((out == NULL) || (in == NULL))
        return;

    for (size_t i = 0; i < ARCHI_SIGNAL_SET_NUM_MASKS; i++)
        out[i] &= in[i];

     // Unset unused bits
    out[ARCHI_SIGNAL_SET_NUM_MASKS - 1] &= archi_signal_set_tail_mask();
}

bool
archi_signal_set_contains(
        archi_signal_set_const_t set,
        int signal_index)
{
    if (set == NULL)
        return false;
    else if ((signal_index < 0) || (signal_index >= ARCHI_SIGNAL_NUMBER + ARCHI_SIGNAL_NUMBER_REALTIME))
        return false;

    return set[signal_index / ARCHI_SIGNAL_SET_MASK_NUM_BITS] &
        ((archi_signal_set_mask_t)1 << (signal_index % ARCHI_SIGNAL_SET_MASK_NUM_BITS));
}

void
archi_signal_set_add(
        archi_signal_set_t set,
        int signal_index)
{
    if (set == NULL)
        return;
    else if ((signal_index < 0) || (signal_index >= ARCHI_SIGNAL_NUMBER + ARCHI_SIGNAL_NUMBER_REALTIME))
        return;

    set[signal_index / ARCHI_SIGNAL_SET_MASK_NUM_BITS] |=
        ((archi_signal_set_mask_t)1 << (signal_index % ARCHI_SIGNAL_SET_MASK_NUM_BITS));
}

void
archi_signal_set_remove(
        archi_signal_set_t set,
        int signal_index)
{
    if (set == NULL)
        return;
    else if ((signal_index < 0) || (signal_index >= ARCHI_SIGNAL_NUMBER + ARCHI_SIGNAL_NUMBER_REALTIME))
        return;

    set[signal_index / ARCHI_SIGNAL_SET_MASK_NUM_BITS] &=
        ~((archi_signal_set_mask_t)1 << (signal_index % ARCHI_SIGNAL_SET_MASK_NUM_BITS));
}

/*****************************************************************************/

archi_signal_flags_t*
archi_signal_flags_alloc(void)
{
    archi_signal_flags_t *signals = malloc(ARCHI_SIZEOF_FLEXIBLE(
                archi_signal_flags_t, rt_signal, ARCHI_SIGNAL_NUMBER_REALTIME));
    if (signals == NULL)
        return NULL;

    for (int i = 0; i < ARCHI_SIGNAL_NUMBER; i++)
        ARCHI_SIGNAL_FLAG_INIT(signals->signal[i]);

    for (int i = 0; i < ARCHI_SIGNAL_NUMBER_REALTIME; i++)
        ARCHI_SIGNAL_FLAG_INIT(signals->rt_signal[i]);

    return signals;
}

