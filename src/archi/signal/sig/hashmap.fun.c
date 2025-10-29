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
 * @brief Signal meta-handler for multiple handler support.
 */

#include "archi/signal/sig/hashmap.fun.h"
#include "archi/signal/sig/hashmap.typ.h"
#include "archi/signal/api/signal.typ.h"
#include "archi/hashmap/api/hashmap.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"

struct archi_hashmap_traverse_data__signal_handler {
    int signo;
    void *siginfo;
    archi_signal_flags_t *flags;

    bool set_signal_flag; // return value
};

static
ARCHI_HASHMAP_TRAV_KV_FUNC(archi_hashmap_traverse__signal_handler)
{
    (void) key;
    (void) index;

    if (!archi_pointer_attr_compatible(value.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_signal_handler_t)))
        return (archi_hashmap_trav_action_t){0};

    struct archi_hashmap_traverse_data__signal_handler *traverse_data = data;
    archi_signal_handler_t *signal_handler = value.ptr;

    if (signal_handler->function != NULL)
    {
        bool set_signal_flag = signal_handler->function(traverse_data->signo,
                traverse_data->siginfo, traverse_data->flags, signal_handler->data);

        traverse_data->set_signal_flag = traverse_data->set_signal_flag || set_signal_flag;
    }

    return (archi_hashmap_trav_action_t){0};
}

ARCHI_SIGNAL_HANDLER_FUNC(archi_signal_handler__hashmap)
{
    archi_signal_handler_data__hashmap_t *hashmap_data = data;

    if (hashmap_data == NULL)
        return true;
    else if (hashmap_data->hashmap == NULL)
        return false;

    struct archi_hashmap_traverse_data__signal_handler traverse_data = {
        .signo = signo,
        .siginfo = siginfo,
        .flags = flags,

        .set_signal_flag = false,
    };

    {
        {
            int ret = mtx_lock(&hashmap_data->hashmap_lock);
            if (ret != thrd_success)
                return false;
        }

        archi_hashmap_traverse(hashmap_data->hashmap, true,
                archi_hashmap_traverse__signal_handler, &traverse_data, NULL);

        mtx_unlock(&hashmap_data->hashmap_lock);
    }

    return traverse_data.set_signal_flag;
}

