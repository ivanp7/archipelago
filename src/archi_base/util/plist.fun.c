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
 * @brief Operations on key-value lists of parameters.
 */

#include "archi_base/util/plist.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"


bool
archi_plist_parse(
        const archi_kvlist_t *params,
        bool with_ref_count,
        archi_plist_param_t parsed[],
        bool ignore_unknown,
        ARCHI_ERROR_PARAM_DECL)
{
    if (parsed == NULL)
    {
        if (!ignore_unknown)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of parameter descriptions is NULL");
            return false;
        }

        ARCHI_ERROR_RESET();
        return true;
    }

    // Iterate over the whole parameter list
    for (; params != NULL; params = params->next)
    {
        if (params->key == NULL)
            continue;

        // Check all parameter descriptions
        for (size_t i = 0; parsed[i].name != NULL; i++)
        {
            // Check the parameter name for equality
            {
                archi_string_comp_func_t comp_fn = parsed[i].comp_fn;
                if (comp_fn == NULL)
                    comp_fn = strcmp;

                if (comp_fn(params->key, parsed[i].name) != 0)
                    continue;
            }

            // Check if the parameter is set already
            if (parsed[i].value_set)
                goto next_parameter;

            // Check the parameter value
            {
                ARCHI_ERROR_VAR(error);

                if (!archi_pointer_valid(params->value, &error))
                {
                    ARCHI_ERROR_SET(error.code, "parameter '%s' has invalid pointer: %s",
                            params->key, error.message);
                    return false;
                }

                if (parsed[i].check.fn != NULL)
                {
                    ARCHI_ERROR_VAR_UNSET(&error);

                    if (!parsed[i].check.fn(params->value, parsed[i].check.data, &error))
                    {
                        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "parameter '%s' has incorrect value: %s",
                                params->key, error.message);
                        return false;
                    }
                }
            }

            // Set the parameter value
            parsed[i].value_set = true;

            if (!with_ref_count)
                parsed[i].value = archi_rcpointer(params->value, NULL);
            else
                parsed[i].value = ((const archi_krcvlist_t*)params)->value;

            // Proceed to the next parameter
            goto next_parameter;
        }

        // Unknown parameter is encountered
        if (!ignore_unknown)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown parameter '%s' encountered", params->key);
            return false;
        }

next_parameter:
        /*nothing*/;
    }

    // Call the assignment functions
    for (size_t i = 0; parsed[i].name != NULL; i++)
    {
        if (!parsed[i].value_set)
            continue;

        if (parsed[i].assign.fn != NULL)
        {
            if (parsed[i].assign.to == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "parameter '%s' has no specified assignment destination",
                        params->key);
                return false;
            }
            else if (parsed[i].assign.sz == 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assignment destination of parameter '%s' has zero size",
                        params->key);
                return false;
            }

            parsed[i].assign.fn(parsed[i].assign.to, parsed[i].assign.sz, parsed[i].value);
        }

        if (parsed[i].assign.flag != NULL)
            *parsed[i].assign.flag = true;
    }

    ARCHI_ERROR_RESET();
    return true;
}

/*****************************************************************************/

ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__bool)
{
    for (size_t i = 0; i < size; i++)
        ((bool*)dest)[i] = ((char*)value.ptr)[i];
}

ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__value)
{
    memcpy(dest, value.ptr, size);
}

ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__dptr)
{
    memcpy(dest, &value.ptr, size);
}

ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__dptr_n)
{
    ARCHI_POINTER_NULLIFY_EMPTY(value);
    archi_plist_assign__dptr(dest, size, value);
}

ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__fptr)
{
    memcpy(dest, &value.fptr, size);
}

ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__pointer)
{
    memcpy(dest, &value.p, size);
}

ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__rcpointer)
{
    memcpy(dest, &value, size);
}

