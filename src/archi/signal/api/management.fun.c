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
 * @brief Signal management operations.
 */

#include "archi/signal/api/management.fun.h"
#include "management.typ.h"

static
struct archi_signal_management_context_base *archi_signal_management;

ARCHI_GLOBAL_SET_FUNC(archi_signal_management_global_context_set)
{
    if (archi_signal_management != NULL)
        return;

    archi_signal_management = context;
}

ARCHI_GLOBAL_GET_FUNC(archi_signal_management_global_context)
{
    return archi_signal_management;
}

archi_signal_flags_t*
archi_signal_management_flags(void)
{
    if (archi_signal_management == NULL)
        return NULL;

    return archi_signal_management->flags;
}

static
bool
archi_signal_management_handler_lock(void)
{
    if (archi_signal_management->signal_handler_lock == NULL)
        return false;

    return thrd_success == mtx_lock(archi_signal_management->signal_handler_lock);
}

static
void
archi_signal_management_handler_unlock(void)
{
    if (archi_signal_management->signal_handler_lock == NULL)
        return;

    mtx_unlock(archi_signal_management->signal_handler_lock);
}

archi_signal_handler_t
archi_signal_management_handler(void)
{
    if (archi_signal_management == NULL)
        return (archi_signal_handler_t){0};

    archi_signal_handler_t signal_handler;
    {
        if (!archi_signal_management_handler_lock())
            return (archi_signal_handler_t){0};

        signal_handler = archi_signal_management->signal_handler;

        archi_signal_management_handler_unlock();
    }

    return signal_handler;
}

void
archi_signal_management_handler_set(
        archi_signal_handler_t signal_handler)
{
    if (archi_signal_management == NULL)
        return;

    {
        if (!archi_signal_management_handler_lock())
            return;

        archi_signal_management->signal_handler = signal_handler;

        archi_signal_management_handler_unlock();
    }
}

