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
 * @brief Aggregate type descriptions for directed execution graph nodes and node arrays.
 */

#pragma once
#ifndef _ARCHI_EXEC_AGG_GRAPH_VAR_H_
#define _ARCHI_EXEC_AGG_GRAPH_VAR_H_

#include "archi/aggr/agg/generic.typ.h"


/**
 * @brief Aggregate type description for archi_dexgraph_transition_t.
 */
extern
const archi_aggr_type_t
archi_aggr_type__dexgraph_transition;

/**
 * @brief Aggregate type description for archi_dexgraph_operation_t.
 */
extern
const archi_aggr_type_t
archi_aggr_type__dexgraph_operation;

/**
 * @brief Aggregate type description for archi_dexgraph_node_t.
 */
extern
const archi_aggr_type_t
archi_aggr_type__dexgraph_node;

/**
 * @brief Aggregate type description for archi_dexgraph_node_array_t.
 */
extern
const archi_aggr_type_t
archi_aggr_type__dexgraph_node_array;

#endif // _ARCHI_EXEC_AGG_GRAPH_VAR_H_

