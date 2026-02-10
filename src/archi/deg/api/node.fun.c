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

#include "archi/deg/api/node.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc(), free()

archi_deg_node_t*
archi_deg_node_alloc(
        size_t sequence_length,

        const archi_deg_node_t *def_branch,
        size_t num_alt_branches,

        archi_deg_branch_function_t branch_fn,
        void *branch_data)
{
    archi_deg_node_t *node = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_deg_node_t, sequence, sequence_length));
    if (node == NULL)
        return NULL;

    if (num_alt_branches != 0)
    {
        node->alt_branch = malloc(sizeof(*node->alt_branch) * num_alt_branches);
        if (node->alt_branch == NULL)
        {
            free(node);
            return NULL;
        }
    }
    else
        node->alt_branch = NULL;

    node->def_branch = def_branch;
    node->num_alt_branches = num_alt_branches;

    node->branch_selector.function = branch_fn;
    node->branch_selector.data = branch_data;

    {
        size_t *sequence_length_ptr = (size_t*)&node->sequence_length;
        *sequence_length_ptr = sequence_length;
    }

    for (size_t i = 0; i < sequence_length; i++)
        node->sequence[i] = (archi_deg_node_func_with_data_t){0};

    for (size_t i = 0; i < num_alt_branches; i++)
        node->alt_branch[i] = NULL;

    return node;
}

void
archi_deg_node_free(
        archi_deg_node_t *node)
{
    if (node == NULL)
        return;

    free(node->alt_branch);
    free(node);
}

