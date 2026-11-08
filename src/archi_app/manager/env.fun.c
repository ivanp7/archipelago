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
 * @brief The environment manager operations.
 */

#include "archi_app/manager/env.fun.h"
#include "../../archi/env/i/manager.typ.h" // for struct archi_env_manager

#include <stdlib.h> // for malloc(), free()


static
struct archi_env_manager archi_manager__env;

ARCHI_MANAGER_PTR_GET_FUNC(archi_app_manager_ptr_get__env)
{
    return &archi_manager__env;
}

bool
archi_app_manager_initialize__env(void)
{
    if (archi_manager__env.initialized)
        return false;

#ifndef __STDC_NO_THREADS__
    archi_manager__env.getenv_lock = malloc(sizeof(*archi_manager__env.getenv_lock));
    if (archi_manager__env.getenv_lock == NULL)
        goto failure_alloc_lock;

    {
        int ret = mtx_init(archi_manager__env.getenv_lock, mtx_plain);
        if (ret != thrd_success)
            goto failure_init_lock;
    }
#endif

    archi_manager__env.initialized = true;
    return true;

#ifndef __STDC_NO_THREADS__
failure_init_lock:
    free(archi_manager__env.getenv_lock);

failure_alloc_lock:
    archi_manager__env = (struct archi_env_manager){0};

    return false;
#endif
}

void
archi_app_manager_finalize__env(void)
{
    if (!archi_manager__env.initialized)
        return;

#ifndef __STDC_NO_THREADS__
    mtx_destroy(archi_manager__env.getenv_lock);
    free(archi_manager__env.getenv_lock);
#endif

    archi_manager__env = (struct archi_env_manager){0};
}

