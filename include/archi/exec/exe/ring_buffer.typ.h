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
 * @brief Data for DEG operation functions for ring buffers operations.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_RING_BUFFER_TYP_H_
#define _ARCHI_EXEC_EXE_RING_BUFFER_TYP_H_

#include <stddef.h> // for size_t


/**
 * @brief Operation function data: circular shift of ring buffer values.
 *
 * Ring buffer must contain `buffer_length + 1` elements.
 * The last element is not a part of the buffer
 * and is used as temporary storage for memory swapping.
 */
typedef struct archi_dexgraph_op_data__ring_buffer_shift {
    void **buffer; ///< Ring buffer.
    size_t buffer_length; ///< Number of elements in the buffer.
    size_t element_size; ///< Size of an element in bytes.
} archi_dexgraph_op_data__ring_buffer_shift_t;

#endif // _ARCHI_EXEC_EXE_RING_BUFFER_TYP_H_

