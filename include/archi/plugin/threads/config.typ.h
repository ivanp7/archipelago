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
 * @brief Context configuration types of the plugin.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_THREADS_CONFIG_TYP_H_
#define _ARCHI_PLUGIN_THREADS_CONFIG_TYP_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Threaded processing configuration.
 */
typedef struct archi_threads_config {
    size_t num_threads; ///< Number of threads to create.
    bool busy_wait;     ///< Whether busy-waiting for a job is enabled.
} archi_threads_config_t;

/**
 * @brief Threaded processing configuration key for the whole configuration structure.
 */
#define ARCHI_THREADS_CONFIG_KEY "config"

/**
 * @brief Threaded processing configuration key -- number of threads to create.
 */
#define ARCHI_THREADS_CONFIG_KEY_NUM_THREADS "num_threads"

/**
 * @brief Threaded processing configuration key -- whether to enable busy-waiting for a job.
 */
#define ARCHI_THREADS_CONFIG_KEY_BUSY_WAIT "busy_wait"

#endif // _ARCHI_PLUGIN_THREADS_CONFIG_TYP_H_

