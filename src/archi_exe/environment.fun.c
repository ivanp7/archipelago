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
 * @brief Operations on the global environment context.
 */

#include "archi_exe/environment.fun.h"
#include "../archi/env/api/context.typ.h" // for struct archi_env_context

#include <stdlib.h> // for malloc(), free()

static
struct archi_env_context archi_environment;

ARCHI_GLOBAL_GET_FUNC(archi_exe_env_global_context)
{
    return &archi_environment;
}

bool
archi_exe_env_initialize(void)
{
    if (archi_environment.initialized)
        return false;

#ifndef __STDC_NO_THREADS__
    archi_environment.getenv_lock = malloc(sizeof(*archi_environment.getenv_lock));
    if (archi_environment.getenv_lock == NULL)
        goto failure_alloc_lock;

    {
        int ret = mtx_init(archi_environment.getenv_lock, mtx_plain);
        if (ret != thrd_success)
            goto failure_init_lock;
    }
#endif

    archi_environment.initialized = true;
    return true;

#ifndef __STDC_NO_THREADS__
failure_init_lock:
    free(archi_environment.getenv_lock);

failure_alloc_lock:
    archi_environment = (struct archi_env_context){0};

    return false;
#endif
}

void
archi_exe_env_finalize(void)
{
    if (!archi_environment.initialized)
        return;

#ifndef __STDC_NO_THREADS__
    mtx_destroy(archi_environment.getenv_lock);
    free(archi_environment.getenv_lock);
#endif

    archi_environment = (struct archi_env_context){0};
}

