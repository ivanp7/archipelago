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
 * @brief DEG operation function for copying memory.
 */

#include "archi/memory/exe/copy.fun.h"
#include "archi/memory/exe/copy.typ.h"

#include <string.h> // for memmove()


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__memory_copy)
{
    const archi_dexgraph_op_data__memory_copy_t *copy = data;

    if (copy == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory copying operation data is NULL");
        return;
    }
    else if (copy->destination == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination memory is NULL");
        return;
    }
    else if (copy->source == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source memory is NULL");
        return;
    }
    else if (copy->stride == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "stride is zero");
        return;
    }

    memmove((char*)copy->destination + copy->offset_dest * copy->stride,
            (const char*)copy->source + copy->offset_src * copy->stride,
            copy->length * copy->stride);

    ARCHI_ERROR_RESET();
}

