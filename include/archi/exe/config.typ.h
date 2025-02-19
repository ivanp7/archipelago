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
 * @brief Application configuration.
 */

#pragma once
#ifndef _ARCHI_EXE_CONFIG_TYP_H_
#define _ARCHI_EXE_CONFIG_TYP_H_

#include "archi/app/instance.typ.h"
#include "archi/util/os/shm.typ.h"

struct archi_signal_watch_set;

/**
 * @brief Configuration of the process in shared memory.
 */
typedef struct archi_process_config_shm {
    archi_shm_header_t shm_header; ///< Shared memory header.

    struct archi_signal_watch_set *signal_watch_set; ///< Signal watch set.
    archi_app_config_t app_config; ///< Application configuration.
} archi_process_config_shm_t;

#endif // _ARCHI_EXE_CONFIG_TYP_H_

