/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Constants of the plugin for operations with threads and concurrent processing.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_THREADS_VTABLE_DEF_H_
#define _ARCHI_PLUGIN_THREADS_VTABLE_DEF_H_

/**
 * @brief Plugin name.
 */
#define ARCHI_PLUGIN_THREADS_NAME "threads"

/**
 * @brief Threads context configuration key -- number of threads to create.
 */
#define ARCHI_THREADS_CONFIG_KEY_NUM_THREADS "num_threads"
/**
 * @brief Threads context configuration key -- whether to enable busy-waiting for a job.
 */
#define ARCHI_THREADS_CONFIG_KEY_BUSY_WAIT "busy_wait"

#endif // _ARCHI_PLUGIN_THREADS_VTABLE_DEF_H_

