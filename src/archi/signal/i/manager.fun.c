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
 * @brief The signal manager operations.
 */

#include "archi/signal/i/manager.fun.h"
#include "manager.typ.h"


static
struct archi_signal_manager_base *archi_signal_manager;

ARCHI_MANAGER_PTR_GET_FUNC(archi_manager_ptr_get__signal)
{
    return archi_signal_manager;
}

ARCHI_MANAGER_PTR_INIT_FUNC(archi_manager_ptr_init__signal)
{
    if (archi_signal_manager != NULL)
        return;

    archi_signal_manager = manager;
}

archi_signal_flags_t*
archi_signal_manager_flags(void)
{
    if (archi_signal_manager == NULL)
        return NULL;

    return archi_signal_manager->flags;
}

static
bool
archi_signal_manager_handler_lock(void)
{
    if (archi_signal_manager->signal_handler_lock == NULL)
        return false;

    return thrd_success == mtx_lock(archi_signal_manager->signal_handler_lock);
}

static
void
archi_signal_manager_handler_unlock(void)
{
    if (archi_signal_manager->signal_handler_lock == NULL)
        return;

    mtx_unlock(archi_signal_manager->signal_handler_lock);
}

archi_signal_handler_t
archi_signal_manager_handler(void)
{
    if (archi_signal_manager == NULL)
        return (archi_signal_handler_t){0};

    archi_signal_handler_t signal_handler;
    {
        if (!archi_signal_manager_handler_lock())
            return (archi_signal_handler_t){0};

        signal_handler = archi_signal_manager->signal_handler;

        archi_signal_manager_handler_unlock();
    }

    return signal_handler;
}

void
archi_signal_manager_handler_set(
        archi_signal_handler_t signal_handler)
{
    if (archi_signal_manager == NULL)
        return;

    {
        if (!archi_signal_manager_handler_lock())
            return;

        archi_signal_manager->signal_handler = signal_handler;

        archi_signal_manager_handler_unlock();
    }
}

