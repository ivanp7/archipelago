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
 * @brief HSP execution context type.
 */

#pragma once
#ifndef _ARCHI_HSP_API_EXEC_TYP_H_
#define _ARCHI_HSP_API_EXEC_TYP_H_

#include <stddef.h> // for size_t

struct archi_hsp_execution_context;

/**
 * @brief Pointer to HSP execution context.
 */
typedef struct archi_hsp_execution_context *archi_hsp_execution_context_t;

#endif // _ARCHI_HSP_API_EXEC_TYP_H_

