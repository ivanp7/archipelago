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
 * @brief Context interface for named value lists used in context interfaces.
 */

#include "archi/app/list.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), memcpy()

ARCHI_CONTEXT_INIT_FUNC(archi_app_value_list_init)
{
    archi_list_t *list = malloc(sizeof(*list));
    if (list == NULL)
        return ARCHI_ERROR_ALLOC;

    *list = (archi_list_t){.head = (archi_list_node_t*)config};

    *context = list;
    *metadata = (void*)config;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_app_value_list_final)
{
    archi_list_t *list = context;

    archi_list_node_t *node = list->head;
    while (node != metadata)
    {
        archi_list_node_t *node_next = node->next;
        archi_list_act_func_free_named(node, 0, NULL);
        node = node_next;
    }

    free(list);
}

ARCHI_CONTEXT_SET_FUNC(archi_app_value_list_set)
{
    (void) metadata;

    archi_list_t *list = context;

    archi_list_node_named_value_t *node = malloc(sizeof(*node));
    if (node == NULL)
        return ARCHI_ERROR_ALLOC;

    *node = (archi_list_node_named_value_t){0};

    node->base.name = archi_list_node_copy_name(slot);
    if (node->base.name == NULL)
    {
        free(node);
        return ARCHI_ERROR_ALLOC;
    }

    node->value = *value;
    node->base.link.next = list->head;

    list->head = (archi_list_node_t*)node;

    return 0;
}

ARCHI_CONTEXT_GET_FUNC(archi_app_value_list_get)
{
    (void) metadata;

    archi_list_t *list = context;

    archi_list_node_named_value_t *node = NULL;
    archi_status_t code = archi_list_traverse(list,
            archi_list_node_func_select_by_name, (void*)slot,
            archi_list_act_func_extract_node, &node, true, 1, NULL);

    if (code < 0)
        return code;
    else if (node == NULL)
        return 1; // not found

    *value = node->value;
    return 0;
}

const archi_context_interface_t archi_app_value_list_interface = {
    .init_fn = archi_app_value_list_init,
    .final_fn = archi_app_value_list_final,
    .set_fn = archi_app_value_list_set,
    .get_fn = archi_app_value_list_get,
};

