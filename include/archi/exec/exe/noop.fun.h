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
 * @brief No-op operation function.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_NOOP_FUN_H_
#define _ARCHI_EXEC_EXE_NOOP_FUN_H_

#include "archi/exec/api/operation.typ.h"


/**
 * @brief Operation function: do nothing.
 *
 * Function data type: <none>.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__noop);

#endif // _ARCHI_EXEC_EXE_NOOP_FUN_H_

