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
 * @brief Context interface for number parsers.
 */

#include "archi/parser/ctx/number.var.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free(), strto*()
#include <limits.h> // *_MAX, *_MIN
#include <errno.h>


enum {
    UNSPECIFIED = 0,

    UNSIGNED_CHAR,
    UNSIGNED_SHORT,
    UNSIGNED_INT,
    UNSIGNED_LONG,
    UNSIGNED_LONG_LONG,

    SIGNED_CHAR,
    SIGNED_SHORT,
    SIGNED_INT,
    SIGNED_LONG,
    SIGNED_LONG_LONG,

    FLOAT,
    DOUBLE,
    LONG_DOUBLE,
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__number_parser)
{
    // Parse parameters
    const char *string = NULL;
    int base = 0;
    int parsed_type = UNSPECIFIED;
    bool base_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "base",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &base, sizeof(base), &base_set}},
            // target types
            {.name = "unsigned_char", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "unsigned_short", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "unsigned_int", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "unsigned_long", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "unsigned_long_long", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "signed_char", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "signed_short", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "signed_int", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "signed_long", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "signed_long_long", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "float", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "double", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {.name = "long_double", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;

        size_t index = 0;

#define PARAMETER(type, base_allowed)   do {    \
        index++;                                \
        if (parsed[index].value_set) {          \
            if (parsed_type != UNSPECIFIED) {   \
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "more than one number type is specified");  \
                return NULL;                    \
            }                                   \
            if (!(base_allowed) && base_set) {  \
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number base is specified, but not accepted for the number type '%s'",  \
                        parsed[index].name);    \
                return NULL;                    \
            }                                   \
            parsed_type = (type);               \
            string = parsed[index].value.ptr;   \
        }                                       \
    } while (0)

        PARAMETER(UNSIGNED_CHAR, true);
        PARAMETER(UNSIGNED_SHORT, true);
        PARAMETER(UNSIGNED_INT, true);
        PARAMETER(UNSIGNED_LONG, true);
        PARAMETER(UNSIGNED_LONG_LONG, true);

        PARAMETER(SIGNED_CHAR, true);
        PARAMETER(SIGNED_SHORT, true);
        PARAMETER(SIGNED_INT, true);
        PARAMETER(SIGNED_LONG, true);
        PARAMETER(SIGNED_LONG_LONG, true);

        PARAMETER(FLOAT, false);
        PARAMETER(DOUBLE, false);
        PARAMETER(LONG_DOUBLE, false);

#undef PARAMETER
    }

    // Check validness of parameters
    if (parsed_type == UNSPECIFIED)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "string and type of a parsed number is not specified");
        return NULL;
    }
    else if (base < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number base (%i) is negative", base);
        return NULL;
    }
    else if (base == 1)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number base (%i) is invalid", base);
        return NULL;
    }
    else if (base > 36)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number base (%i) is unsupported", base);
        return NULL;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    // Parse the string
#define _OUT_OF_RANGE() do {                                                \
        free(context_data);                                                 \
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number out of type range");    \
        return NULL;                                                        \
    } while (0)

#define _OUT_OF_MEMORY() do {                                               \
        free(context_data);                                                 \
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate number memory"); \
        return NULL;                                                        \
    } while (0)

#define _INIT_CONTEXT_DATA(type, value) do {                \
        *context_data = (archi_rcpointer_t){                \
            .ptr = malloc(sizeof(type)),                    \
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |     \
                    ARCHI_POINTER_ATTR__PDATA(1, type),     \
        };                                                  \
        if (context_data->ptr == NULL)                      \
            _OUT_OF_MEMORY();                               \
        *(type*)context_data->ptr = (type)(value);          \
    } while (0)

    errno = 0;

    switch (parsed_type)
    {
#define PARSE_INTEGER_MINMAX(type, conv_fn, ret_type, minval, maxval) {         \
        ret_type parsed = conv_fn(string, NULL, base);                          \
        if ((errno == ERANGE) || (parsed < (minval)) || (parsed > (maxval)))    \
            _OUT_OF_RANGE();                                                    \
        _INIT_CONTEXT_DATA(type, parsed);                                       \
        break; }

#define PARSE_INTEGER_MAX(type, conv_fn, ret_type, maxval) {    \
        ret_type parsed = conv_fn(string, NULL, base);          \
        if ((errno == ERANGE) || (parsed > (maxval)))           \
            _OUT_OF_RANGE();                                    \
        _INIT_CONTEXT_DATA(type, parsed);                       \
        break; }

#define PARSE_INTEGER(type, conv_fn, ret_type) {        \
        ret_type parsed = conv_fn(string, NULL, base);  \
        if (errno == ERANGE)                            \
            _OUT_OF_RANGE();                            \
        _INIT_CONTEXT_DATA(type, parsed);               \
        break; }

#define PARSE_FLOAT(type, conv_fn) {            \
        type parsed = conv_fn(string, NULL);    \
        if (errno == ERANGE)                            \
            _OUT_OF_RANGE();                            \
        _INIT_CONTEXT_DATA(type, parsed);               \
        break; }

        case UNSIGNED_CHAR: PARSE_INTEGER_MAX(unsigned char, strtoul, unsigned long, UCHAR_MAX)
        case UNSIGNED_SHORT: PARSE_INTEGER_MAX(unsigned short, strtoul, unsigned long, USHRT_MAX)
        case UNSIGNED_INT: PARSE_INTEGER_MAX(unsigned int, strtoul, unsigned long, UINT_MAX)
        case UNSIGNED_LONG: PARSE_INTEGER(unsigned long, strtoul, unsigned long)
        case UNSIGNED_LONG_LONG: PARSE_INTEGER(unsigned long long, strtoull, unsigned long long)

        case SIGNED_CHAR: PARSE_INTEGER_MINMAX(signed char, strtol, signed long, SCHAR_MIN, SCHAR_MAX)
        case SIGNED_SHORT: PARSE_INTEGER_MINMAX(signed short, strtol, signed long, SHRT_MIN, SHRT_MAX)
        case SIGNED_INT: PARSE_INTEGER_MINMAX(signed int, strtol, signed long, INT_MIN, INT_MAX)
        case SIGNED_LONG: PARSE_INTEGER(signed long, strtol, signed long)
        case SIGNED_LONG_LONG: PARSE_INTEGER(signed long long, strtoll, signed long long)

        case FLOAT: PARSE_FLOAT(float, strtof)
        case DOUBLE: PARSE_FLOAT(double, strtod)
        case LONG_DOUBLE: PARSE_FLOAT(long double, strtold)
    }

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__number_parser)
{
    free(context->ptr);
    free(context);
}

const archi_context_interface_t
archi_context_interface__number_parser = {
    .init_fn = archi_context_init__number_parser,
    .final_fn = archi_context_final__number_parser,
};

