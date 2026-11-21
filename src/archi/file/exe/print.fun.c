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
 * @brief Operation functions for printing formatted messages to streams.
 */

#include "archi/file/exe/print.fun.h"
#include "archi/file/exe/print.typ.h"
#include "archi/file/exe/tag.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"

#include <stdio.h> // for stdout, fprintf()
#include <wchar.h> // for wint_t
#include <stddef.h> // for size_t, ptrdiff_t


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__file_printf)
{
    archi_error_t error;
    if (!archi_pointer_attr_compatible(data.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_OP_DATA__FILE_PRINTF),
                &error))
    {
        ARCHI_ERROR_SET(error.code, "operation data has incorrect type: %s", error.message);
        return;
    }

    const archi_dexgraph_op_data__file_printf_t *print_data = data.cptr;

    if (print_data == NULL)
    {
        ARCHI_ERROR_RESET();
        return;
    }

    FILE *stream = print_data->stream;
    if (stream == NULL)
        stream = stdout;

    for (size_t i = 0; i < print_data->num_tokens; i++)
    {
        archi_dexgraph_op_data__file_printf__token_t token = print_data->token[i];

        if (token.conv_spec == NULL)
            continue;
        else if (token.conv_spec[0] != '%')
        {
            fprintf(stream, "%s", token.conv_spec);
            continue;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(token.argument.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "conversion specification argument #%zu is not a data pointer", i);
            return;
        }

        size_t length = strlen(token.conv_spec);
        if (length < 2)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "conversion specification is too short (less than 2 characters)");
            return;
        }

        char conversion_specifier = token.conv_spec[length - 1];
        char length_modifier = token.conv_spec[length - 2];
        char length_modifier2 = (length >= 3) ? token.conv_spec[length - 3] : '%';

        switch (conversion_specifier)
        {
#define CHECK_TYPE(type) do {                                                                                           \
            if (!archi_pointer_attr_compatible(token.argument.attr,                                                     \
                        ARCHI_POINTER_ATTR__PDATA(1, type), &error)) {                                                  \
                ARCHI_ERROR_SET(error.code, "attributes of conversion specification argument #%zu are incorrect: %s",   \
                        i, error.message);                                                                              \
                return; }                                                                                               \
            else if (token.argument.ptr == NULL) {                                                                      \
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to conversion specification argument #%zu is NULL", i);    \
                return; }                                                                                               \
            else if (!archi_pointer_valid(token.argument, &error)) {                                                    \
                ARCHI_ERROR_SET(error.code, "pointer to conversion specification argument #%zu is invalid: %s",         \
                        i, error.message);                                                                              \
                return; }                                                                                               \
        } while (0)

            case '%':
                if (token.argument.ptr != NULL)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to conversion specification argument #%zu is not NULL", i);
                    return;
                }
                fprintf(stream, token.conv_spec);
                break;

            case 'c':
                switch (length_modifier)
                {
                    default:
                        CHECK_TYPE(int);
                        fprintf(stream, token.conv_spec, *(int*)token.argument.ptr);
                        break;

                    case 'l':
                        CHECK_TYPE(wint_t);
                        fprintf(stream, token.conv_spec, *(wint_t*)token.argument.ptr);
                        break;
                }
                break;

            case 's':
                switch (length_modifier)
                {
                    default:
                        CHECK_TYPE(char);
                        fprintf(stream, token.conv_spec, (char*)token.argument.ptr);
                        break;

                    case 'l':
                        CHECK_TYPE(wchar_t);
                        fprintf(stream, token.conv_spec, (wchar_t*)token.argument.ptr);
                        break;
                }
                break;

            case 'd':
            case 'i':
                switch (length_modifier)
                {
                    case 'h':
                        if (length_modifier2 == 'h')
                        {
                            CHECK_TYPE(signed char);
                            fprintf(stream, token.conv_spec, *(signed char*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(short);
                            fprintf(stream, token.conv_spec, *(short*)token.argument.ptr);
                        }
                        break;

                    default:
                        CHECK_TYPE(int);
                        fprintf(stream, token.conv_spec, *(int*)token.argument.ptr);
                        break;

                    case 'l':
                        if (length_modifier2 == 'l')
                        {
                            CHECK_TYPE(long long);
                            fprintf(stream, token.conv_spec, *(long long*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(long);
                            fprintf(stream, token.conv_spec, *(long*)token.argument.ptr);
                        }
                        break;

                    case 'j':
                        CHECK_TYPE(intmax_t);
                        fprintf(stream, token.conv_spec, *(intmax_t*)token.argument.ptr);
                        break;

                    case 'z':
                        CHECK_TYPE(size_t);
                        fprintf(stream, token.conv_spec, *(size_t*)token.argument.ptr);
                        break;

                    case 't':
                        CHECK_TYPE(ptrdiff_t);
                        fprintf(stream, token.conv_spec, *(ptrdiff_t*)token.argument.ptr);
                        break;
                }
                break;

            case 'u':
            case 'o':
            case 'x':
            case 'X':
                switch (length_modifier)
                {
                    case 'h':
                        if (length_modifier2 == 'h')
                        {
                            CHECK_TYPE(unsigned char);
                            fprintf(stream, token.conv_spec, *(unsigned char*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(unsigned short);
                            fprintf(stream, token.conv_spec, *(unsigned short*)token.argument.ptr);
                        }
                        break;

                    default:
                        CHECK_TYPE(unsigned int);
                        fprintf(stream, token.conv_spec, *(unsigned int*)token.argument.ptr);
                        break;

                    case 'l':
                        if (length_modifier2 == 'l')
                        {
                            CHECK_TYPE(unsigned long long);
                            fprintf(stream, token.conv_spec, *(unsigned long long*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(unsigned long);
                            fprintf(stream, token.conv_spec, *(unsigned long*)token.argument.ptr);
                        }
                        break;

                    case 'j':
                        CHECK_TYPE(uintmax_t);
                        fprintf(stream, token.conv_spec, *(uintmax_t*)token.argument.ptr);
                        break;

                    case 'z':
                        CHECK_TYPE(size_t);
                        fprintf(stream, token.conv_spec, *(size_t*)token.argument.ptr);
                        break;

                    case 't':
                        CHECK_TYPE(ptrdiff_t);
                        fprintf(stream, token.conv_spec, *(ptrdiff_t*)token.argument.ptr);
                        break;
                }
                break;

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'a':
            case 'A':
                switch (length_modifier)
                {
                    default:
                        CHECK_TYPE(double);
                        fprintf(stream, token.conv_spec, *(double*)token.argument.ptr);
                        break;

                    case 'l':
                        CHECK_TYPE(double);
                        fprintf(stream, token.conv_spec, *(double*)token.argument.ptr);
                        break;

                    case 'L':
                        CHECK_TYPE(long double);
                        fprintf(stream, token.conv_spec, *(long double*)token.argument.ptr);
                        break;
                }
                break;

            case 'p':
                fprintf(stream, token.conv_spec, token.argument.ptr);
                break;

            case 'n':
                if (!ARCHI_POINTER_TO_WRITABLE_DATA(token.argument.attr))
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "conversion specification argument #%zu is not writable", i);
                    return;
                }

                switch (length_modifier)
                {
                    case 'h':
                        if (length_modifier2 == 'h')
                        {
                            CHECK_TYPE(signed char);
                            fprintf(stream, token.conv_spec, (signed char*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(short);
                            fprintf(stream, token.conv_spec, (short*)token.argument.ptr);
                        }
                        break;

                    default:
                        CHECK_TYPE(int);
                        fprintf(stream, token.conv_spec, (int*)token.argument.ptr);
                        break;

                    case 'l':
                        if (length_modifier2 == 'l')
                        {
                            CHECK_TYPE(long long);
                            fprintf(stream, token.conv_spec, (long long*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(long);
                            fprintf(stream, token.conv_spec, (long*)token.argument.ptr);
                        }
                        break;

                    case 'j':
                        CHECK_TYPE(intmax_t);
                        fprintf(stream, token.conv_spec, (intmax_t*)token.argument.ptr);
                        break;

                    case 'z':
                        CHECK_TYPE(size_t);
                        fprintf(stream, token.conv_spec, (size_t*)token.argument.ptr);
                        break;

                    case 't':
                        CHECK_TYPE(ptrdiff_t);
                        fprintf(stream, token.conv_spec, (ptrdiff_t*)token.argument.ptr);
                        break;
                }
                break;

            default:
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "conversion specifier '%c' is not supported",
                        conversion_specifier);
                return;

#undef CHECK_TYPE
        }
    }

    ARCHI_ERROR_RESET();
}

