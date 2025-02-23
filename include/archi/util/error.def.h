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
 * @brief Standard error codes.
 */

#pragma once
#ifndef _ARCHI_UTIL_ERROR_DEF_H_
#define _ARCHI_UTIL_ERROR_DEF_H_

#define ARCHI_ERROR_UNKNOWN     -1  ///< Unknown error.
#define ARCHI_ERROR_MISUSE      -2  ///< Error: incorrect use of an interface (incorrect arguments such as null pointers and out-of-range values).
#define ARCHI_ERROR_CONFIG      -3  ///< Error: incorrect configuration provided.
#define ARCHI_ERROR_INIT        -4  ///< Error: couldn't perform initialization (of a resource).
#define ARCHI_ERROR_OPERATION   -5  ///< Error: couldn't perform an operation.
#define ARCHI_ERROR_ALLOC       -6  ///< Error: couldn't allocate memory.
#define ARCHI_ERROR_MAP         -7  ///< Error: couldn't map memory.
#define ARCHI_ERROR_LOAD        -8  ///< Error: couldn't load shared library.
#define ARCHI_ERROR_SYMBOL      -9  ///< Error: couldn't find a symbol in shared library.
#define ARCHI_ERROR_INTERFACE   -10 ///< Error: required function is not available in interface.
#define ARCHI_ERROR_RESOURCE    -11 ///< Error: required resource is not available.
#define ARCHI_ERROR_FORMAT      -12 ///< Error: data format is incorrect.

#define ARCHI_ERROR_USER        -16 ///< First error code reserved for user.

/**
 * @brief Offset of application error exit codes.
 */
#define ARCHI_EXIT_CODE_BASE 64

/**
 * @brief Calculate application exit code from error code.
 */
#define ARCHI_EXIT_CODE(code) (((code) >= 0) ? (code) : ARCHI_EXIT_CODE_BASE - (code))

#endif // _ARCHI_UTIL_ERROR_DEF_H_

