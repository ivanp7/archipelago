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
#define ARCHI_ERROR_ALLOC       -4  ///< Error: couldn't allocate memory.
#define ARCHI_ERROR_ATTACH      -5  ///< Error: couldn't attach shared memory.
#define ARCHI_ERROR_LOAD        -6  ///< Error: couldn't load shared library.
#define ARCHI_ERROR_SYMBOL      -7  ///< Error: couldn't find a symbol in shared library.
#define ARCHI_ERROR_FORMAT      -8  ///< Error: virtual table format is incorrect (or wrong vtable pointer).
#define ARCHI_ERROR_FUNCTION    -9  ///< Error: required function is not available.
#define ARCHI_ERROR_SIGNAL      -10 ///< Error: couldn't initialize signal management.
#define ARCHI_ERROR_THREAD      -11 ///< Error: couldn't create thread.
#define ARCHI_ERROR_FILE        -12 ///< Error: couldn't open file.
#define ARCHI_ERROR_USER        -13 ///< First error code reserved for user.

/**
 * @brief Offset of application error exit codes.
 */
#define ARCHI_EXIT_CODE_BASE 64

/**
 * @brief Calculate application exit code from error code.
 */
#define ARCHI_EXIT_CODE(code) (((code) >= 0) ? (code) : ARCHI_EXIT_CODE_BASE - (code))

#endif // _ARCHI_UTIL_ERROR_DEF_H_

