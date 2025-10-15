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

#pragma once
#ifndef _ARCHI_MEM_HSP_COPY_FUN_H_
#define _ARCHI_MEM_HSP_COPY_FUN_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief State function for calling archi_memory_map_copy_unmap().
 *
 * This state function expects archi_memory_map_copy_unmap_data_t
 * object as function data.
 */
ARCHI_HSP_STATE_FUNCTION(archi_hsp_state_memory_map_copy_unmap);

#endif // _ARCHI_MEM_HSP_COPY_FUN_H_

