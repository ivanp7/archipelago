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
 * @brief DEG operation functions for ring buffers operations.
 */

#include "archi/exec/exe/ring_buffer.fun.h"
#include "archi/exec/exe/ring_buffer.typ.h"

#include <string.h> // for memcpy()


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__ring_buffer_shift)
{
    const archi_dexgraph_op_data__ring_buffer_shift_t *shift_data = data;

    if (shift_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "ring buffer shift data is NULL");
        return;
    }
    else if (shift_data->buffer == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "ring buffer is NULL");
        return;
    }
    else if (shift_data->element_size == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "ring buffer element size is zero");
        return;
    }

    for (size_t i = 0; i < shift_data->buffer_length + 1; i++)
    {
        if (shift_data->buffer[i] == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "ring buffer element #%zu is NULL", i);
            return;
        }
    }

    for (size_t i = shift_data->buffer_length; i > 0; i--)
        memcpy(shift_data->buffer[i], shift_data->buffer[i - 1], shift_data->element_size);

    memcpy(shift_data->buffer[0], shift_data->buffer[shift_data->buffer_length], shift_data->element_size);

    ARCHI_ERROR_RESET();
}

