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
 * @brief The logger context type.
 */

#pragma once
#ifndef _ARCHIPELAGO_LOG_CONTEXT_TYP_H_
#define _ARCHIPELAGO_LOG_CONTEXT_TYP_H_

#include <stdio.h> // for FILE
#include <time.h> // for struct timespec
#include <stdbool.h>
#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h> // for atomic_flag
#endif

struct archi_log_context {
    FILE *stream; ///< Stream to print messages to.

    struct timespec start_time; ///< Application start time.
    int verbosity_level;        ///< Log verbosity level.
    bool colorful;              ///< Whether color is used in log messages.

#ifndef __STDC_NO_ATOMICS__
    atomic_flag spinlock; ///< Spinlock for message printing atomicity.
#endif
};

#endif // _ARCHIPELAGO_LOG_CONTEXT_TYP_H_

