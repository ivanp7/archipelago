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
 * @brief Array of descriptions of built-in global objects.
 */

#include "archi_base/global.var.h"
#include "archi_app/logging.fun.h"
#include "archi_app/signal_management.fun.h"
#include "archi_app/environment.fun.h"


const archi_global_init_spec_t
archi_global_init_specs[] = {
    {.get_fn = archi_app_log_global_context, .set_fn_symbol = "archi_global_context_set__log"},
    {.get_fn = archi_app_signal_management_global_context, .set_fn_symbol = "archi_global_context_set__signal_management"},
    {.get_fn = archi_app_env_global_context, .set_fn_symbol = "archi_global_context_set__env"},
    {0},
};

