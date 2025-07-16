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

#include "archi/builtin/convert/string_to_number.var.h"

#include <stdlib.h> // for malloc(), free(), strto*()
#include <string.h> // for strcmp()
#include <limits.h> // *_MAX, *_MIN
#include <errno.h>
#include <stdbool.h>
#include <stdalign.h>

struct archi_context_converter_string_to_number_data {
    archi_pointer_t number;

    union {
        unsigned char as_uchar;
        unsigned short as_ushort;
        unsigned long as_ulong;
        unsigned long long as_ulonglong;

        signed char as_schar;
        signed short as_sshort;
        signed long as_slong;
        signed long long as_slonglong;

        float as_float;
        double as_double;
        long double as_longdouble;
    };
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_converter_string_to_number_init)
{
    struct archi_context_converter_string_to_number_data value = {0};

    if ((params == NULL) || (params->next != NULL))
        return ARCHI_STATUS_EKEY;
    else if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (params->value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    errno = 0;

    if (strcmp("as_uchar", params->name) == 0)
    {
        unsigned long parsed = strtoul(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed > UCHAR_MAX))
            return ARCHI_STATUS_EVALUE;

        value.as_uchar = (unsigned char)parsed;
        value.number = (archi_pointer_t){
            .ptr = &value.as_uchar,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_uchar),
                .alignment = alignof(unsigned char),
            },
        };
    }
    else if (strcmp("as_ushort", params->name) == 0)
    {
        unsigned long parsed = strtoul(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed > USHRT_MAX))
            return ARCHI_STATUS_EVALUE;

        value.as_ushort = (unsigned short)parsed;
        value.number = (archi_pointer_t){
            .ptr = &value.as_ushort,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_ushort),
                .alignment = alignof(unsigned short),
            },
        };
    }
    else if (strcmp("as_ulong", params->name) == 0)
    {
        value.as_ulong = strtoul(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        value.number = (archi_pointer_t){
            .ptr = &value.as_ulong,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_ulong),
                .alignment = alignof(unsigned long),
            },
        };
    }
    else if (strcmp("as_ulonglong", params->name) == 0)
    {
        value.as_ulonglong = strtoull(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        value.number = (archi_pointer_t){
            .ptr = &value.as_ulonglong,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_ulonglong),
                .alignment = alignof(unsigned long long),
            },
        };
    }
    else if (strcmp("as_schar", params->name) == 0)
    {
        signed long parsed = strtol(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed < SCHAR_MIN) || (parsed > SCHAR_MAX))
            return ARCHI_STATUS_EVALUE;

        value.as_schar = (signed char)parsed;
        value.number = (archi_pointer_t){
            .ptr = &value.as_schar,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_schar),
                .alignment = alignof(signed char),
            },
        };
    }
    else if (strcmp("as_sshort", params->name) == 0)
    {
        signed long parsed = strtol(params->value.ptr, NULL, 0);
        if ((errno == ERANGE) || (parsed < SHRT_MIN) || (parsed > SHRT_MAX))
            return ARCHI_STATUS_EVALUE;

        value.as_sshort = (signed short)parsed;
        value.number = (archi_pointer_t){
            .ptr = &value.as_sshort,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_sshort),
                .alignment = alignof(signed short),
            },
        };
    }
    else if (strcmp("as_slong", params->name) == 0)
    {
        value.as_slong = strtol(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        value.number = (archi_pointer_t){
            .ptr = &value.as_slong,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_slong),
                .alignment = alignof(signed long),
            },
        };
    }
    else if (strcmp("as_slonglong", params->name) == 0)
    {
        value.as_slonglong = strtoll(params->value.ptr, NULL, 0);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        value.number = (archi_pointer_t){
            .ptr = &value.as_slonglong,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_slonglong),
                .alignment = alignof(signed long long),
            },
        };
    }
    else if (strcmp("as_float", params->name) == 0)
    {
        value.as_float = strtof(params->value.ptr, NULL);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        value.number = (archi_pointer_t){
            .ptr = &value.as_float,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_float),
                .alignment = alignof(float),
            },
        };
    }
    else if (strcmp("as_double", params->name) == 0)
    {
        value.as_double = strtod(params->value.ptr, NULL);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        value.number = (archi_pointer_t){
            .ptr = &value.as_double,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_double),
                .alignment = alignof(double),
            },
        };
    }
    else if (strcmp("as_longdouble", params->name) == 0)
    {
        value.as_longdouble = strtold(params->value.ptr, NULL);
        if (errno == ERANGE)
            return ARCHI_STATUS_EVALUE;

        value.number = (archi_pointer_t){
            .ptr = &value.as_longdouble,
            .element = {
                .num_of = 1,
                .size = sizeof(value.as_longdouble),
                .alignment = alignof(long double),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    struct archi_context_converter_string_to_number_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *context_data = value;

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_converter_string_to_number_final)
{
    free(context);
}

const archi_context_interface_t archi_context_converter_string_to_number_interface = {
    .init_fn = archi_context_converter_string_to_number_init,
    .final_fn = archi_context_converter_string_to_number_final,
};

