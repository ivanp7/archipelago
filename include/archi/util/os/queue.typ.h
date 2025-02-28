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
#ifndef _ARCHI_UTIL_OS_QUEUE_TYP_H_
#define _ARCHI_UTIL_OS_QUEUE_TYP_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Parameters for archi_queue_alloc().
 */
typedef struct archi_queue_config {
    size_t capacity_log2; ///< Log2 of maximum capacity of queue.

    size_t element_alignment_log2; ///< Log2 of queue element alignment in bytes.
    size_t element_size; ///< Queue element size in bytes.
} archi_queue_config_t;

/**
 * @brief Lock-less queue configuration key for the whole configuration structure.
 */
#define ARCHI_QUEUE_CONFIG_KEY "config"

/**
 * @brief Lock-less queue configuration key -- log2 of maximum capacity of queue.
 */
#define ARCHI_QUEUE_CONFIG_KEY_CAPACITY_LOG2 "capacity_log2"

/**
 * @brief Lock-less queue configuration key -- log2 of queue element alignment in bytes.
 */
#define ARCHI_QUEUE_CONFIG_KEY_ELEMENT_ALIGNMENT_LOG2 "element_alignment_log2"

/**
 * @brief Lock-less queue configuration key -- queue element size in bytes.
 */
#define ARCHI_QUEUE_CONFIG_KEY_ELEMENT_SIZE "element_size"

#endif // _ARCHI_UTIL_OS_QUEUE_TYP_H_

