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
 * @brief Types for lock-free queue operations.
 */

#pragma once
#ifndef _ARCHI_DS_LFQUEUE_API_LFQUEUE_TYP_H_
#define _ARCHI_DS_LFQUEUE_API_LFQUEUE_TYP_H_

#include <stddef.h> // for size_t

/**
 * @brief Parameters for archi_lfqueue_alloc().
 *
 * Element size can be zero, in that case no data is stored in the queue,
 * only numbers of pushes and pops are counted.
 *
 * Element alignment requirement must be a power of two.
 */
typedef struct archi_lfqueue_alloc_params {
    size_t capacity_log2; ///< Log base 2 of maximum capacity of queue.

    size_t element_size;      ///< Queue element size in bytes.
    size_t element_alignment; ///< Queue element alignment requirement in bytes.
} archi_lfqueue_alloc_params_t;

#endif // _ARCHI_DS_LFQUEUE_API_LFQUEUE_TYP_H_

