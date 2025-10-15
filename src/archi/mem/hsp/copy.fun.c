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
 * @brief HSP state for copying data between memory objects.
 */

#include "archi/mem/hsp/copy.fun.h"
#include "archi/mem/hsp/copy.typ.h"
#include "archi/mem/api/interface.fun.h"
#include "archi/hsp/api/state.fun.h"
#include "archipelago/log/print.fun.h"

ARCHI_HSP_STATE_FUNCTION(archi_hsp_state_memory_map_copy_unmap)
{
#define M "archi_hsp_state_memory_map_copy_unmap"

    archi_memory_map_copy_unmap_data_t *copy_data = ARCHI_HSP_CURRENT_STATE().data;
    if (copy_data == NULL)
        return;

    archi_status_t code = archi_memory_map_copy_unmap(
            copy_data->memory_dest, copy_data->offset_dest, copy_data->map_data_dest,
            copy_data->memory_src, copy_data->offset_src, copy_data->map_data_src,
            copy_data->num_of);

    if (code != 0)
        archi_log_error(M, "archi_memory_map_copy_unmap() -> %i", code);

#undef M
}

