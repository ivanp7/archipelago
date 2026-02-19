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
 * @brief Operation functions for printing messages.
 */

#include "archi/exec/exe/print.fun.h"
#include "archi/exec/exe/print.typ.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"

#include <stdio.h> // for printf()
#include <wchar.h> // for wint_t
#include <stddef.h> // for size_t, ptrdiff_t


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__print_string)
{
    if (data == NULL)
    {
        ARCHI_ERROR_RESET();
        return;
    }

    const archi_dexgraph_op_data__print_string_t *print_data = data;

    for (size_t i = 0; i < print_data->num_tokens; i++)
    {
        archi_dexgraph_op_data__print_string__token_t token = print_data->token[i];

        if (token.conv_spec == NULL)
            continue;
        else if (token.conv_spec[0] != '%')
        {
            printf("%s", token.conv_spec);
            continue;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(token.argument.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "printf argument is not a data pointer");
            return;
        }

        size_t length = strlen(token.conv_spec);
        if (length < 2)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "printf conversion specification is too short");
            return;
        }

        char conversion_specifier = token.conv_spec[length - 1];
        char length_modifier = token.conv_spec[length - 2];
        char length_modifier2 = (length >= 3) ? token.conv_spec[length - 3] : '%';

        switch (conversion_specifier)
        {
#define CHECK_TYPE(type) do {                                                                       \
            if (!archi_pointer_attr_compatible(token.argument.attr,                                 \
                        ARCHI_POINTER_ATTR__PDATA(1, type))) {                                      \
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "printf argument attributes are incorrect");    \
                return; }                                                                           \
            else if (token.argument.ptr == NULL) {                                                  \
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "printf argument pointer is NULL");             \
                return; }                                                                           \
            else if (!archi_pointer_valid(token.argument, NULL)) {                                  \
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "printf argument pointer is invalid");          \
                return; }                                                                           \
        } while (0)

            case '%':
                if (token.argument.ptr != NULL)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "printf argument pointer is not NULL");
                    return;
                }
                printf(token.conv_spec);
                break;

            case 'c':
                switch (length_modifier)
                {
                    default:
                        CHECK_TYPE(int);
                        printf(token.conv_spec, *(int*)token.argument.ptr);
                        break;

                    case 'l':
                        CHECK_TYPE(wint_t);
                        printf(token.conv_spec, *(wint_t*)token.argument.ptr);
                        break;
                }
                break;

            case 's':
                switch (length_modifier)
                {
                    default:
                        CHECK_TYPE(char);
                        printf(token.conv_spec, (char*)token.argument.ptr);
                        break;

                    case 'l':
                        CHECK_TYPE(wchar_t);
                        printf(token.conv_spec, (wchar_t*)token.argument.ptr);
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
                            printf(token.conv_spec, *(signed char*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(short);
                            printf(token.conv_spec, *(short*)token.argument.ptr);
                        }
                        break;

                    default:
                        CHECK_TYPE(int);
                        printf(token.conv_spec, *(int*)token.argument.ptr);
                        break;

                    case 'l':
                        if (length_modifier2 == 'l')
                        {
                            CHECK_TYPE(long long);
                            printf(token.conv_spec, *(long long*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(long);
                            printf(token.conv_spec, *(long*)token.argument.ptr);
                        }
                        break;

                    case 'j':
                        CHECK_TYPE(intmax_t);
                        printf(token.conv_spec, *(intmax_t*)token.argument.ptr);
                        break;

                    case 'z':
                        CHECK_TYPE(size_t);
                        printf(token.conv_spec, *(size_t*)token.argument.ptr);
                        break;

                    case 't':
                        CHECK_TYPE(ptrdiff_t);
                        printf(token.conv_spec, *(ptrdiff_t*)token.argument.ptr);
                        break;
                }
                break;

            case 'o':
            case 'x':
            case 'X':
            case 'u':
                switch (length_modifier)
                {
                    case 'h':
                        if (length_modifier2 == 'h')
                        {
                            CHECK_TYPE(unsigned char);
                            printf(token.conv_spec, *(unsigned char*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(unsigned short);
                            printf(token.conv_spec, *(unsigned short*)token.argument.ptr);
                        }
                        break;

                    default:
                        CHECK_TYPE(unsigned int);
                        printf(token.conv_spec, *(unsigned int*)token.argument.ptr);
                        break;

                    case 'l':
                        if (length_modifier2 == 'l')
                        {
                            CHECK_TYPE(unsigned long long);
                            printf(token.conv_spec, *(unsigned long long*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(unsigned long);
                            printf(token.conv_spec, *(unsigned long*)token.argument.ptr);
                        }
                        break;

                    case 'j':
                        CHECK_TYPE(uintmax_t);
                        printf(token.conv_spec, *(uintmax_t*)token.argument.ptr);
                        break;

                    case 'z':
                        CHECK_TYPE(size_t);
                        printf(token.conv_spec, *(size_t*)token.argument.ptr);
                        break;

                    case 't':
                        CHECK_TYPE(ptrdiff_t);
                        printf(token.conv_spec, *(ptrdiff_t*)token.argument.ptr);
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
                        printf(token.conv_spec, *(double*)token.argument.ptr);
                        break;

                    case 'l':
                        CHECK_TYPE(double);
                        printf(token.conv_spec, *(double*)token.argument.ptr);
                        break;

                    case 'L':
                        CHECK_TYPE(long double);
                        printf(token.conv_spec, *(long double*)token.argument.ptr);
                        break;
                }
                break;

            case 'p':
                printf(token.conv_spec, token.argument.ptr);
                break;

            case 'n':
                switch (length_modifier)
                {
                    case 'h':
                        if (length_modifier2 == 'h')
                        {
                            CHECK_TYPE(signed char);
                            printf(token.conv_spec, (signed char*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(short);
                            printf(token.conv_spec, (short*)token.argument.ptr);
                        }
                        break;

                    default:
                        CHECK_TYPE(int);
                        printf(token.conv_spec, (int*)token.argument.ptr);
                        break;

                    case 'l':
                        if (length_modifier2 == 'l')
                        {
                            CHECK_TYPE(long long);
                            printf(token.conv_spec, (long long*)token.argument.ptr);
                        }
                        else
                        {
                            CHECK_TYPE(long);
                            printf(token.conv_spec, (long*)token.argument.ptr);
                        }
                        break;

                    case 'j':
                        CHECK_TYPE(intmax_t);
                        printf(token.conv_spec, (intmax_t*)token.argument.ptr);
                        break;

                    case 'z':
                        CHECK_TYPE(size_t);
                        printf(token.conv_spec, (size_t*)token.argument.ptr);
                        break;

                    case 't':
                        CHECK_TYPE(ptrdiff_t);
                        printf(token.conv_spec, (ptrdiff_t*)token.argument.ptr);
                        break;
                }
                break;

            default:
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "printf conversion specifier '%c' is not supported",
                        conversion_specifier);
                return;

#undef CHECK_TYPE
        }
    }

    ARCHI_ERROR_RESET();
}

