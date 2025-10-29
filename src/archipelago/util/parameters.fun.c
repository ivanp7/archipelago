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

#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"

bool
archi_kvlist_parameters_parse(
        const archi_kvlist_rc_t *params,
        archi_kvlist_parameter_t parsed[],
        size_t num_parsed,
        bool ignore_unknown,
        archi_string_comparison_func_t comp_fn,
        ARCHI_ERROR_PARAMETER_DECL)
{
    if ((parsed == NULL) && (num_parsed > 0))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of parameter descriptions is NULL");
        return false;
    }

    if (comp_fn == NULL)
        comp_fn = strcmp;

    for (; params != NULL; params = params->next)
    {
        for (size_t i = 0; i < num_parsed; i++)
        {
            if (comp_fn(params->key, parsed[i].name) != 0)
                continue;

            if (!parsed[i].value_set)
            {
                if (!parsed[i].value_unchecked &&
                        !archi_pointer_attr_compatible(params->value.attr, parsed[i].value.attr))
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "parameter '%s' has incompatible attributes",
                            params->key);
                    return false;
                }

                archi_error_t error;
                if (!archi_pointer_valid(params->value.p, &error))
                {
                    ARCHI_ERROR_SET(error.code, "parameter '%s' has invalid pointer: %s",
                            params->key, error.message);
                    return false;
                }

                parsed[i].value = params->value;
                parsed[i].value_set = true;
            }

            goto next_parameter;
        }

        if (!ignore_unknown)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown parameter '%s' encountered", params->key);
            return false;
        }

next_parameter:
        /*noop*/;
    }

    ARCHI_ERROR_RESET();
    return true;
}

