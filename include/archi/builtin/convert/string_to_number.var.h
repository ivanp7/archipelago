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

#pragma once
#ifndef _ARCHI_BUILTIN_CONVERT_STRING_TO_NUMBER_VAR_H_
#define _ARCHI_BUILTIN_CONVERT_STRING_TO_NUMBER_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_converter_string_to_number_init);   ///< String-to-number converter initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_converter_string_to_number_final); ///< String-to-number converter finalization function.

extern
const archi_context_interface_t archi_context_converter_string_to_number_interface; ///< String-to-number converter interface.

#endif // _ARCHI_BUILTIN_CONVERT_STRING_TO_NUMBER_VAR_H_

