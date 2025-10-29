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
 * @brief Implementation of environment interface.
 */

#include "archi/env/api/context.fun.h"
#include "archi/env/api/variable.fun.h"
#include "archipelago/util/string.fun.h"
#include "context.typ.h" // for struct archi_env_context

#include <stdlib.h> // for getenv()

static
struct archi_env_context *archi_environment;

ARCHI_GLOBAL_SET_FUNC(archi_env_global_context_set)
{
    if (archi_environment != NULL)
        return;

    archi_environment = context;
}

ARCHI_GLOBAL_GET_FUNC(archi_env_global_context)
{
    return archi_environment;
}

char*
archi_env_get(
        const char *name,
        ARCHI_ERROR_PARAMETER_DECL)
{
    if (name == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "env. variable name is NULL");
        return NULL;
    }

    char *value = NULL;

#ifndef __STDC_NO_THREADS__
    if ((archi_environment != NULL) && (archi_environment->getenv_lock != NULL))
    {
        int ret = mtx_lock(archi_environment->getenv_lock);
        if (ret != thrd_success)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't lock getenv() mutex");
            return NULL;
        }
    }
#endif

    const char *value_orig = getenv(name);
    if (value_orig == NULL) // env. variable not found
    {
        ARCHI_ERROR_RESET();
        goto finish;
    }

    value = archi_string_copy(value_orig);
    if (value == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate string copy");
        goto finish;
    }

    ARCHI_ERROR_RESET();

finish:
#ifndef __STDC_NO_THREADS__
    if ((archi_environment != NULL) && (archi_environment->getenv_lock != NULL))
        mtx_unlock(archi_environment->getenv_lock);
#endif

    return value;
}

