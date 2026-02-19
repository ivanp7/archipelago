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
 * @brief Signal meta-handler for multiple handler support.
 */

#pragma once
#ifndef _ARCHI_SIGNAL_SIG_HASHMAP_FUN_H_
#define _ARCHI_SIGNAL_SIG_HASHMAP_FUN_H_

#include "archi/signal/api/handler.typ.h"


/**
 * @brief Signal meta-handler for multiple handler support.
 *
 * Function data type: archi_signal_handler_hashmap_data_t.
 *
 * Hashmap values must be signal handler objects only.
 * When a signal is caught, it is passed to all handlers in the hashmap
 * in the order of oldest to newest.
 */
ARCHI_SIGNAL_HANDLER_FUNC(archi_signal_handler__hashmap);

#endif // _ARCHI_SIGNAL_SIG_HASHMAP_FUN_H_

