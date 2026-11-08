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
 * @brief The array of descriptions of built-in managers.
 */

#include "archi_base/manager.var.h"
#include "archi_app/manager/log.fun.h"
#include "archi_app/manager/signal.fun.h"
#include "archi_app/manager/env.fun.h"


const archi_manager_ptr_init_spec_t
archi_manager_ptr_init_specs[] = {
    {"archi_manager_ptr_init__log", archi_app_manager_ptr_get__log},
    {"archi_manager_ptr_init__signal", archi_app_manager_ptr_get__signal},
    {"archi_manager_ptr_init__env", archi_app_manager_ptr_get__env},
    {0},
};

