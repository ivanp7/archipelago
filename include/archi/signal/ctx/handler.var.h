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
 * @brief Context interface for the signal handlers.
 */

#pragma once
#ifndef _ARCHI_SIGNAL_CTX_HANDLER_VAR_H_
#define _ARCHI_SIGNAL_CTX_HANDLER_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: signal handler.
 *
 * Initialization parameters:
 * - "function" : (archi_signal_handler_function_t) signal handler function
 * - "data"     : signal handler data
 *
 * Getter slots:
 * - "function" : (archi_signal_handler_function_t) signal handler function
 * - "data"     : signal handler data
 *
 * Setter slots:
 * - "function" : (archi_signal_handler_function_t) signal handler function
 * - "data"     : signal handler data
 */
extern
const archi_context_interface_t
archi_context_interface__signal_handler;

#endif // _ARCHI_SIGNAL_CTX_HANDLER_VAR_H_

