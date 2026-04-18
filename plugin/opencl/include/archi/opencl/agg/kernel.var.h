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
 * @brief Aggregate type descriptions for data of DEG operation functions for OpenCL kernel operations.
 */

#pragma once
#ifndef _ARCHI_OPENCL_AGG_KERNEL_VAR_H_
#define _ARCHI_OPENCL_AGG_KERNEL_VAR_H_

#include "archi/aggr/agg/generic.typ.h"


/**
 * @brief Aggregate type description for archi_dexgraph_op_data__opencl_kernel_enqueue_t.
 */
extern
const archi_aggr_type_t
archi_aggr_type__dexgraph_op_data__opencl_kernel_enqueue;

#endif // _ARCHI_OPENCL_AGG_KERNEL_VAR_H_

