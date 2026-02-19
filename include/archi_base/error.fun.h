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
 * @brief Error handling operations.
 */

#pragma once
#ifndef _ARCHI_BASE_ERROR_FUN_H_
#define _ARCHI_BASE_ERROR_FUN_H_

#include "archi_base/error.typ.h"


/**
 * @brief Get error code string.
 *
 * @return Error code string, or NULL for non-standard codes.
 */
const char*
archi_error_code_string(
        archi_error_code_t code ///< [in] Error code.
);

#endif // _ARCHI_BASE_ERROR_FUN_H_

