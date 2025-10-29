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
 * @brief Type for error handling.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_ERROR_TYP_H_
#define _ARCHIPELAGO_BASE_ERROR_TYP_H_

#include <limits.h> // for INT_MIN
#include <stdio.h> // for snprintf()
#include <string.h> // for strcpy()
#include <stdlib.h> // for abort()

/**
 * @brief Error code type.
 *
 * Meaning of error codes:
 *   - code = 0 : success
 *   - code < 0 : standard error codes
 *   - code > 0 : non-standard error codes reserved for user
 */
typedef int archi_error_code_t;

/*****************************************************************************/

#define ARCHI__EFAILURE     ((archi_error_code_t)-1)       ///< Generic/unknown failure.

#define ARCHI__ECONTRACT    ((archi_error_code_t)-2)       ///< Contract violated (incorrect API usage).
#define ARCHI__ECONSTRAINT  ((archi_error_code_t)-3)       ///< Constraint violated (incorrect value).

#define ARCHI__EKEY         ((archi_error_code_t)-4)       ///< Invalid key encountered.
#define ARCHI__EINDEX       ((archi_error_code_t)-5)       ///< Index out of bounds.

#define ARCHI__EMEMORY      ((archi_error_code_t)-6)       ///< Memory allocation failure.
#define ARCHI__ESYSTEM      ((archi_error_code_t)-7)       ///< System resource/operation failure.

#define ARCHI__ENOTIMPL     ((archi_error_code_t)INT_MIN)  ///< Feature or function is not implemented.

/*****************************************************************************/

/**
 * @brief Size of an error message buffer.
 */
#define ARCHI_ERROR_MESSAGE_BUFFER_SIZE     ((size_t)1 << 10)   // 1 KiB

/**
 * @brief Error description.
 */
typedef struct archi_error {
    archi_error_code_t code; ///< Error code.
    char message[ARCHI_ERROR_MESSAGE_BUFFER_SIZE]; ///< Error message.

    struct {
        const char *func; ///< Function name.
        const char *file; ///< File name.
        int line;         ///< Line number.
    } origin; ///< Origin of the error.
} archi_error_t;

/*****************************************************************************/

/**
 * @brief Error parameter identifier.
 */
#define ARCHI_ERROR_PARAMETER       archi_parameter_error

/**
 * @brief Error parameter declaration for function parameter lists.
 */
#define ARCHI_ERROR_PARAMETER_DECL  archi_error_t *const restrict ARCHI_ERROR_PARAMETER

/*****************************************************************************/

/**
 * @brief Fill error variable with error info.
 */
#define ARCHI_ERROR_SET_VAR(ptr, err_code, ...) do {                            \
    if ((ptr) != NULL) {                                                        \
        (ptr)->code = (err_code);                                               \
        snprintf((ptr)->message, ARCHI_ERROR_MESSAGE_BUFFER_SIZE, __VA_ARGS__)  \
        < 0 ? abort() : (void)0; /* suppress GCC -Wformat-truncation warning */ \
        (ptr)->origin.func = __func__;                                          \
        (ptr)->origin.file = __FILE__;                                          \
        (ptr)->origin.line = __LINE__;                                          \
    }                                                                           \
} while (0)

/**
 * @brief Reset error info in a variable.
 */
#define ARCHI_ERROR_RESET_VAR(ptr)  do {    \
    if ((ptr) != NULL) {                    \
        (ptr)->code = 0;                    \
        (ptr)->message[0] = '\0';           \
        (ptr)->origin.func = NULL;          \
        (ptr)->origin.file = NULL;          \
        (ptr)->origin.line = 0;             \
    }                                       \
} while (0)

/**
 * @brief Assign error variable to another.
 */
#define ARCHI_ERROR_ASSIGN_VAR(ptr, err)    do {    \
    if ((ptr) != NULL) {                            \
        (ptr)->code = (err).code;                   \
        strcpy((ptr)->message, (err).message);      \
        (ptr)->origin = (err).origin;               \
    }                                               \
} while (0)

/*****************************************************************************/

/**
 * @brief Fill the error parameter with error info.
 */
#define ARCHI_ERROR_SET(err_code, ...)  \
    ARCHI_ERROR_SET_VAR(ARCHI_ERROR_PARAMETER, (err_code), __VA_ARGS__)

/**
 * @brief Reset error info in the parameter.
 */
#define ARCHI_ERROR_RESET() \
    ARCHI_ERROR_RESET_VAR(ARCHI_ERROR_PARAMETER)

/**
 * @brief Assign error to the error parameter.
 */
#define ARCHI_ERROR_ASSIGN(err) \
    ARCHI_ERROR_ASSIGN_VAR(ARCHI_ERROR_PARAMETER, (err))

#endif // _ARCHIPELAGO_BASE_ERROR_TYP_H_

