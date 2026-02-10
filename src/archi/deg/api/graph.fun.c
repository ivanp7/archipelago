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
 * @brief Directed execution graph operations.
 */

#include "archi/deg/api/graph.fun.h"

void
archi_deg_execute(
        const archi_deg_node_t *node,
        ARCHI_ERROR_PARAMETER_DECL)
{
    archi_error_t error;
    ARCHI_ERROR_RESET_VAR(&error);

    while (node != NULL)
    {
        // Execute the sequence of node functions
        for (size_t i = 0; i < node->sequence_length; i++)
        {
            archi_deg_node_func_with_data_t seq_elt = node->sequence[i];

            if (seq_elt.function == NULL)
                continue;

            /*************************************/
            seq_elt.function(seq_elt.data, &error);
            /*************************************/

            if (error.code != 0)
                goto terminate;
        }

        // Select a branch
        size_t index;

        if (node->branch_selector.function != NULL)
            index = node->branch_selector.function(node->branch_selector.data);
        else if (node->branch_selector.data != NULL)
            index = *(size_t*)node->branch_selector.data;
        else
            index = 0;

        // Proceed to the next node or terminate
        if (index == ARCHI_DEG_TERMINATE)
            goto terminate;
        else if (index == 0)
            node = node->def_branch;
        else if ((index <= node->num_alt_branches) && (node->alt_branch != NULL))
            node = node->alt_branch[index - 1];
        else
            goto terminate;
    }

terminate:
    ARCHI_ERROR_ASSIGN(error);
}

