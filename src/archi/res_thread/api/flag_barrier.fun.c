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
 * @brief Operations with flag barriers.
 */

#include "archi/res_thread/api/flag_barrier.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <threads.h> // for cnd_*, mtx_*
#include <stdbool.h>

struct archi_thread_flag_barrier {
    bool flag;
    bool sense;

    cnd_t condition_variable;
    mtx_t mutex;
};

archi_thread_flag_barrier_t
archi_thread_flag_barrier_alloc(
        archi_status_t *code)
{
    archi_thread_flag_barrier_t barrier = malloc(sizeof(*barrier));
    if (barrier == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    *barrier = (struct archi_thread_flag_barrier){0};

    int err;

    err = cnd_init(&barrier->condition_variable);
    if (err != thrd_success)
    {
        if (code != NULL)
        {
            if (err == thrd_nomem)
                *code = ARCHI_STATUS_ENOMEMORY;
            else
                *code = ARCHI_STATUS_ERESOURCE;
        }

        free(barrier);
        return NULL;
    }

    err = mtx_init(&barrier->mutex, mtx_plain);
    if (err != thrd_success)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        cnd_destroy(&barrier->condition_variable);
        free(barrier);
        return NULL;
    }

    return barrier;
}

void
archi_thread_flag_barrier_destroy(
        archi_thread_flag_barrier_t barrier)
{
    if (barrier == NULL)
        return;

    mtx_destroy(&barrier->mutex);
    cnd_destroy(&barrier->condition_variable);

    free(barrier);
}

archi_status_t
archi_thread_flag_barrier_acquire(
        archi_thread_flag_barrier_t barrier)
{
    if (barrier == NULL)
        return 0;

    int err;

    err = mtx_lock(&barrier->mutex);
    if (err != thrd_success)
        return ARCHI_STATUS_ERESOURCE;

    archi_status_t code = 0;

    if (barrier->flag != barrier->sense)
    {
        code = ARCHI_STATUS_EMISUSE;
        goto finish;
    }

    barrier->sense = !barrier->sense;

finish:
    err = mtx_unlock(&barrier->mutex);
    if (err != thrd_success)
        return ARCHI_STATUS_ERESOURCE;

    return code;
}

archi_status_t
archi_thread_flag_barrier_release(
        archi_thread_flag_barrier_t barrier)
{
    if (barrier == NULL)
        return 0;

    int err;

    err = mtx_lock(&barrier->mutex);
    if (err != thrd_success)
        return ARCHI_STATUS_ERESOURCE;

    archi_status_t code = 0;

    if (barrier->flag == barrier->sense)
    {
        code = ARCHI_STATUS_EMISUSE;
        goto finish;
    }

    barrier->flag = barrier->sense;

    err = cnd_broadcast(&barrier->condition_variable);
    if (err != thrd_success)
        code = ARCHI_STATUS_ERESOURCE;

finish:
    err = mtx_unlock(&barrier->mutex);
    if (err != thrd_success)
        return ARCHI_STATUS_ERESOURCE;

    return code;
}

archi_status_t
archi_thread_flag_barrier_wait(
        archi_thread_flag_barrier_t barrier)
{
    if (barrier == NULL)
        return 0;

    int err;

    err = mtx_lock(&barrier->mutex);
    if (err != thrd_success)
        return ARCHI_STATUS_ERESOURCE;

    archi_status_t code = 0;

    while (barrier->flag != barrier->sense)
    {
        err = cnd_wait(&barrier->condition_variable, &barrier->mutex);
        if (err != thrd_success)
        {
            code = ARCHI_STATUS_ERESOURCE;
            goto finish;
        }
    }

finish:
    err = mtx_unlock(&barrier->mutex);
    if (err != thrd_success)
        return ARCHI_STATUS_ERESOURCE;

    return code;
}

