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
 * @brief Signal management context base.
 */

#pragma once
#ifndef _SRC_ARCHI_SIGNAL_API_MANAGEMENT_TYP_H_
#define _SRC_ARCHI_SIGNAL_API_MANAGEMENT_TYP_H_

#include "archi/signal/api/signal.typ.h"
#include "archi/signal/api/handler.typ.h"

#include <threads.h> // for mtx_t

struct archi_signal_management_context_base {
    archi_signal_flags_t *flags;

    archi_signal_handler_t signal_handler;
    mtx_t *signal_handler_lock;
};

#endif // _SRC_ARCHI_SIGNAL_API_MANAGEMENT_TYP_H_

