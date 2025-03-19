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

// incorrect input
#define ARCHI_ERROR_MISUSE      -2  ///< Error: incorrect use of an interface (forbidden value passed).
#define ARCHI_ERROR_CONFIG      -3  ///< Error: incorrect configuration provided.
#define ARCHI_ERROR_FORMAT      -4  ///< Error: data format is incorrect.

// resource unavailability
#define ARCHI_ERROR_INTERFACE   -5  ///< Error: required function (method) is not available in interface.
#define ARCHI_ERROR_UNAVAIL     -6  ///< Error: resource in not available.
#define ARCHI_ERROR_SYMBOL      -7  ///< Error: couldn't find a symbol in shared library.

// operation failure
#define ARCHI_ERROR_OPERATION   -8  ///< Error: couldn't perform an operation.
#define ARCHI_ERROR_ALLOC       -9  ///< Error: couldn't allocate memory.
#define ARCHI_ERROR_OPEN        -10 ///< Error: couldn't open file.
#define ARCHI_ERROR_MAP         -11 ///< Error: couldn't map memory.
#define ARCHI_ERROR_LOAD        -12 ///< Error: couldn't load shared library.

// user error code base
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

