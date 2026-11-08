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
 * @brief The logging manager operations.
 */

#include "archi_app/manager/log.fun.h"
#include "archi_log/verbosity.def.h"
#include "../../archi_log/manager.typ.h" // for struct archi_log_manager

#include <stdlib.h> // for malloc(), free()


static
struct archi_log_manager archi_manager__log;

ARCHI_MANAGER_PTR_GET_FUNC(archi_app_manager_ptr_get__log)
{
    return &archi_manager__log;
}

bool
archi_app_manager_initialize__log(
        FILE *stream,
        int level,
        int indentation,
        bool colorful)
{
    if (archi_manager__log.stream != NULL)
        return false;

    // Set logging stream
    archi_manager__log.stream = (stream != NULL) ? stream : stderr;

    // Store logging start time
    timespec_get(&archi_manager__log.start_time, TIME_UTC);

    // Set logging verbosity
    if (level < 0)
        level = 0;
    else if (level > ARCHI_LOG_VERBOSITY_MAX)
        level = ARCHI_LOG_VERBOSITY_MAX;

    archi_manager__log.verbosity_level = level;

    // Set indentation width
    if (indentation < 0)
        indentation = 0;

    archi_manager__log.indentation_width = indentation;

    // Set logging color use flag
    archi_manager__log.colorful = colorful;

#ifndef __STDC_NO_THREADS__
    // Allocate and initialize logging mutex
    archi_manager__log.lock = malloc(sizeof(*archi_manager__log.lock));
    if (archi_manager__log.lock == NULL)
        goto failure_alloc_lock;

    {
        int ret = mtx_init(archi_manager__log.lock, mtx_recursive);
        if (ret != thrd_success)
            goto failure_init_lock;
    }
#endif

    return true;

#ifndef __STDC_NO_THREADS__
failure_init_lock:
    free(archi_manager__log.lock);

failure_alloc_lock:
    archi_manager__log = (struct archi_log_manager){0};

    return false;
#endif
}

void
archi_app_manager_finalize__log(void)
{
    if (archi_manager__log.stream == NULL)
        return;

#ifndef __STDC_NO_THREADS__
    mtx_destroy(archi_manager__log.lock);
    free(archi_manager__log.lock);
#endif

    archi_manager__log = (struct archi_log_manager){0};
}

