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

#pragma once
#ifndef _ARCHI_PARSER_CTX_NUMBER_VAR_H_
#define _ARCHI_PARSER_CTX_NUMBER_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: number parser.
 *
 * Initialization parameters:
 * - "base" : (int) base of a parsed integer
 * and exactly one of:
 * - "unsigned_char"        : (char[]) string parsed as unsigned char
 * - "unsigned_short"       : (char[]) string parsed as unsigned short
 * - "unsigned_int"         : (char[]) string parsed as unsigned int
 * - "unsigned_long"        : (char[]) string parsed as unsigned long
 * - "unsigned_long_long"   : (char[]) string parsed as unsigned long long
 * - "signed_char"          : (char[]) string parsed as signed char
 * - "signed_short"         : (char[]) string parsed as signed short
 * - "signed_int"           : (char[]) string parsed as signed int
 * - "signed_long"          : (char[]) string parsed as signed long
 * - "signed_long_long"     : (char[]) string parsed as signed long long
 * - "float"                : (char[]) string parsed as float
 * - "double"               : (char[]) string parsed as double
 * - "long_double"          : (char[]) string parsed as long double
 */
extern
const archi_context_interface_t
archi_context_interface__number_parser;

#endif // _ARCHI_PARSER_CTX_NUMBER_VAR_H_

