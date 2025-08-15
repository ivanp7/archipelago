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
 * @brief Application context interface for string-to-number converters.
 */

#include "archi/util_convert/ctx/string_to_number.var.h"

#include <stdlib.h> // for malloc(), free(), strto*()
#include <string.h> // for strcmp()
#include <limits.h> // *_MAX, *_MIN
#include <errno.h>
#include <stdbool.h>
#include <stdalign.h>

ARCHI_CONTEXT_INIT_FUNC(archi_context_convert_string_to_number_init)
{
    archi_pointer_t value = {0};

    if ((params == NULL) || (params->next != NULL))
        return ARCHI_STATUS_EKEY;
    else if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (params->value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    errno = 0;

#define INIT_VALUE(type) do {           \
    value = (archi_pointer_t){          \
        .ptr = malloc(sizeof(type)),    \
        .element = {                    \
            .num_of = 1,                \
            .size = sizeof(type),       \
            .alignment = alignof(type), \
        },                              \
    };                                  \
    if (value.ptr == NULL)              \
        return ARCHI_STATUS_ENOMEMORY;  \
    *(type*)value.ptr = (type)parsed;   \
} while (0)

    if (strcmp("as_uchar", params->name) == 0)
    {
        unsigned long parsed = strtoul(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed > UCHAR_MAX))
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(unsigned char);
    }
    else if (strcmp("as_ushort", params->name) == 0)
    {
        unsigned long parsed = strtoul(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed > USHRT_MAX))
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(unsigned short);
    }
    else if (strcmp("as_uint", params->name) == 0)
    {
        unsigned long parsed = strtoul(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed > UINT_MAX))
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(unsigned int);
    }
    else if (strcmp("as_ulong", params->name) == 0)
    {
        unsigned long parsed = strtoul(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(unsigned long);
    }
    else if (strcmp("as_ulonglong", params->name) == 0)
    {
        unsigned long long parsed = strtoull(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(unsigned long long);
    }
    else if (strcmp("as_schar", params->name) == 0)
    {
        signed long parsed = strtol(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed < SCHAR_MIN) || (parsed > SCHAR_MAX))
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(signed char);
    }
    else if (strcmp("as_sshort", params->name) == 0)
    {
        signed long parsed = strtol(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed < SHRT_MIN) || (parsed > SHRT_MAX))
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(signed short);
    }
    else if (strcmp("as_sint", params->name) == 0)
    {
        signed long parsed = strtol(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed < INT_MIN) || (parsed > INT_MAX))
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(signed int);
    }
    else if (strcmp("as_slong", params->name) == 0)
    {
        signed long parsed = strtol(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(signed long);
    }
    else if (strcmp("as_slonglong", params->name) == 0)
    {
        signed long long parsed = strtoll(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(signed long long);
    }
    else if (strcmp("as_float", params->name) == 0)
    {
        float parsed = strtof(params->value.ptr, NULL);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(float);
    }
    else if (strcmp("as_double", params->name) == 0)
    {
        double parsed = strtod(params->value.ptr, NULL);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(double);
    }
    else if (strcmp("as_longdouble", params->name) == 0)
    {
        long double parsed = strtold(params->value.ptr, NULL);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        INIT_VALUE(long double);
    }
    else
        return ARCHI_STATUS_EKEY;

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        free(value.ptr);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *context_data = value;

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_convert_string_to_number_final)
{
    free(context->ptr);
    free(context);
}

const archi_context_interface_t archi_context_convert_string_to_number_interface = {
    .init_fn = archi_context_convert_string_to_number_init,
    .final_fn = archi_context_convert_string_to_number_final,
};

