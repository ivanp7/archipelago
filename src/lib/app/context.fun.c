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
 * @brief Operations on contexts.
 */

#include "archi/app/context.fun.h"
#include "archi/app/context.typ.h"
#include "archi/util/value.typ.h"
#include "archi/util/error.def.h"

archi_status_t
archi_context_initialize(
        archi_context_t *context,

        const struct archi_list_node_named_value *config)
{
    if ((context == NULL) || (context->interface == NULL))
        return ARCHI_ERROR_MISUSE;

    context->handle = NULL;

    if (context->interface->init_fn != NULL)
        return context->interface->init_fn(&context->handle, config);
    else
        return 0;
}

void
archi_context_finalize(
        archi_context_t *context)
{
    if ((context == NULL) || (context->interface == NULL))
        return;

    if (context->interface->final_fn != NULL)
        context->interface->final_fn(context->handle);

    context->handle = NULL;
}

archi_status_t
archi_context_set(
        archi_context_t *context,

        const char *slot,
        const archi_value_t *value)
{
    if ((context == NULL) || (context->interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if ((slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (context->interface->set_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    return context->interface->set_fn(&context->handle, slot, value);
}

archi_status_t
archi_context_get(
        archi_context_t *context,

        const char *slot,
        archi_value_t *value)
{
    if ((context == NULL) || (context->interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if ((slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (context->interface->get_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    return context->interface->get_fn(&context->handle, slot, value);
}

archi_status_t
archi_context_assign(
        archi_context_t *dest,
        const char *dest_slot,

        archi_context_t *src,
        const char *src_slot)
{
    if ((dest == NULL) || (dest->interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (dest_slot == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (dest->interface->set_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    if ((src == NULL) || (src->interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if ((src_slot != NULL) && (src->interface->get_fn == NULL))
        return ARCHI_ERROR_INTERFACE;

    archi_value_t value = {0};

    if (src_slot != NULL)
    {
        archi_status_t code = src->interface->get_fn(&src->handle, src_slot, &value);
        if (code != 0)
            return code;
    }
    else
    {
        value.ptr = src->handle;
        value.num_of = 1;
        value.type = ARCHI_VALUE_DATA;
    }

    return dest->interface->set_fn(&dest->handle, dest_slot, &value);
}

archi_status_t
archi_context_act(
        archi_context_t *context,

        const char *action,
        const struct archi_list_node_named_value *params)
{
    if ((context == NULL) || (context->interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (action == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (context->interface->act_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    return context->interface->act_fn(&context->handle, action, params);
}

