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
 * @brief Operations on the global log context.
 */

#include "archi_app/logging.fun.h"
#include "archi_log/verbosity.def.h"
#include "../archi_log/context.typ.h" // for struct archi_log_context

#include <stdlib.h> // for malloc(), free()


static
struct archi_log_context archi_logger;

ARCHI_GLOBAL_GET_FUNC(archi_app_log_global_context)
{
    return &archi_logger;
}

bool
archi_app_log_initialize(
        FILE *stream,
        int level,
        bool colorful)
{
    if (archi_logger.stream != NULL)
        return false;

    // Set logging stream
    archi_logger.stream = (stream != NULL) ? stream : stderr;

    // Store logging start time
    timespec_get(&archi_logger.start_time, TIME_UTC);

    // Set logging verbosity
    if (level < 0)
        level = 0;
    else if (level > ARCHI_LOG_VERBOSITY_MAX)
        level = ARCHI_LOG_VERBOSITY_MAX;

    archi_logger.verbosity_level = level;

    // Set logging color use flag
    archi_logger.colorful = colorful;

#ifndef __STDC_NO_THREADS__
    // Allocate and initialize logging mutex
    archi_logger.lock = malloc(sizeof(*archi_logger.lock));
    if (archi_logger.lock == NULL)
        goto failure_alloc_lock;

    {
        int ret = mtx_init(archi_logger.lock, mtx_recursive);
        if (ret != thrd_success)
            goto failure_init_lock;
    }
#endif

    return true;

#ifndef __STDC_NO_THREADS__
failure_init_lock:
    free(archi_logger.lock);

failure_alloc_lock:
    archi_logger = (struct archi_log_context){0};

    return false;
#endif
}

void
archi_app_log_finalize(void)
{
    if (archi_logger.stream == NULL)
        return;

#ifndef __STDC_NO_THREADS__
    mtx_destroy(archi_logger.lock);
    free(archi_logger.lock);
#endif

    archi_logger = (struct archi_log_context){0};
}

