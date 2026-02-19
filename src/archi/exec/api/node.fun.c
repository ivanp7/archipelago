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
 * @brief Functions for directed execution graph nodes.
 */

#include "archi/exec/api/node.fun.h"
#include "archi_base/util/size.def.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


archi_dexgraph_node_t*
archi_dexgraph_node_alloc(
        const char *name,
        size_t sequence_length)
{
    archi_dexgraph_node_t *node = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_dexgraph_node_t, sequence, sequence_length));
    if (node == NULL)
        return NULL;

    node->name = archi_string_copy(name);
    if ((node->name == NULL) && (name != NULL))
    {
        free(node);
        return NULL;
    }

    node->transition = (archi_dexgraph_transition_t){0};
    node->branch = NULL;
    {
        size_t *sequence_length_ptr = (size_t*)&node->sequence_length;
        *sequence_length_ptr = sequence_length;
    }
    for (size_t i = 0; i < sequence_length; i++)
        node->sequence[i] = (archi_dexgraph_operation_t){0};

    return node;
}

void
archi_dexgraph_node_free(
        archi_dexgraph_node_t *node)
{
    if (node == NULL)
        return;

    free(node->name);
    free(node);
}

archi_dexgraph_node_array_t*
archi_dexgraph_node_array_alloc(
        size_t num_nodes)
{
    archi_dexgraph_node_array_t *node_array = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_dexgraph_node_array_t, node, num_nodes));
    if (node_array == NULL)
        return NULL;

    {
        size_t *num_nodes_ptr = (size_t*)&node_array->num_nodes;
        *num_nodes_ptr = num_nodes;
    }
    for (size_t i = 0; i < num_nodes; i++)
        node_array->node[i] = NULL;

    return node_array;
}

void
archi_dexgraph_node_array_free(
        archi_dexgraph_node_array_t *node_array)
{
    if (node_array == NULL)
        return;

    free(node_array);
}

