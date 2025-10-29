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
 * @brief Finish-on-error hierarchical state processor state.
 */

#pragma once
#ifndef _ARCHI_HSP_HSP_FINISH_STATE_FUN_H_
#define _ARCHI_HSP_HSP_FINISH_STATE_FUN_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief State function for finishing HSP execution on non-zero error code.
 *
 * State function data is not used.
 */
ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__finish_on_error);

#endif // _ARCHI_HSP_HSP_FINISH_STATE_FUN_H_

