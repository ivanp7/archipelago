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
#ifndef _ARCHI_BASE_ERROR_TYP_H_
#define _ARCHI_BASE_ERROR_TYP_H_

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

#define ARCHI__EFAILURE     ((archi_error_code_t)-1)        ///< Generic/unknown failure.

#define ARCHI__ECONSTRAINT  ((archi_error_code_t)-2)        ///< Constraint violated.
#define ARCHI__ECONTRACT    ((archi_error_code_t)-3)        ///< Contract violated.
#define ARCHI__ENOTIMPL     ((archi_error_code_t)-4)        ///< Feature or function is not implemented.

#define ARCHI__EKEY         ((archi_error_code_t)-8)        ///< Invalid key encountered.
#define ARCHI__EINDEX       ((archi_error_code_t)-9)        ///< Index out of bounds.

#define ARCHI__EMEMORY      ((archi_error_code_t)-16)       ///< Memory allocation failure.
#define ARCHI__ESYSTEM      ((archi_error_code_t)-17)       ///< System resource/operation failure.

#define ARCHI__EUNSPECIFIED ((archi_error_code_t)INT_MIN)   ///< Error code has not been explicitly set.

/*****************************************************************************/

/**
 * @brief Size of an error message buffer.
 */
#define ARCHI_ERROR_MESSAGE_SIZE    (1 << 7) // 128

/**
 * @brief Error description.
 */
typedef struct archi_error {
    archi_error_code_t code; ///< Error code.

    struct {
        int line;         ///< Line number.
        const char *file; ///< File name.
        const char *func; ///< Function name.
    } origin; ///< Origin of the error.

    char message[ARCHI_ERROR_MESSAGE_SIZE]; ///< Error message.
} archi_error_t;

/*****************************************************************************/

/**
 * @brief Define and initialize an error variable (without message buffer).
 */
#define ARCHI_ERROR_VAR(var)    \
    archi_error_t var;          \
    ARCHI_ERROR_VAR_UNSET(&var)

/**
 * @brief Fill error variable with error info.
 */
#define ARCHI_ERROR_VAR_SET(ptr, err_code, ...) do {                            \
    if ((ptr) != NULL) {                                                        \
        (ptr)->code = (err_code);                                               \
        (ptr)->origin.line = __LINE__;                                          \
        (ptr)->origin.file = __FILE__;                                          \
        (ptr)->origin.func = __func__;                                          \
        snprintf((ptr)->message, ARCHI_ERROR_MESSAGE_SIZE, __VA_ARGS__)         \
        < 0 ? abort() : (void)0; /* suppress GCC -Wformat-truncation warning */ \
    }                                                                           \
} while (0)

/**
 * @brief Unset error info in a variable.
 */
#define ARCHI_ERROR_VAR_UNSET(ptr)  do {    \
    if ((ptr) != NULL) {                    \
        (ptr)->code = ARCHI__EUNSPECIFIED;  \
        (ptr)->origin.line = __LINE__;      \
        (ptr)->origin.file = __FILE__;      \
        (ptr)->origin.func = __func__;      \
        (ptr)->message[0] = '\0';           \
    }                                       \
} while (0)

/**
 * @brief Set success status to error variable.
 */
#define ARCHI_ERROR_VAR_RESET(ptr)  do {    \
    if ((ptr) != NULL) {                    \
        (ptr)->code = 0;                    \
        (ptr)->origin.line = __LINE__;      \
        (ptr)->origin.file = __FILE__;      \
        (ptr)->origin.func = __func__;      \
        (ptr)->message[0] = '\0';           \
    }                                       \
} while (0)

/**
 * @brief Assign error variable to another.
 */
#define ARCHI_ERROR_VAR_ASSIGN(ptr, err)    do {    \
    if ((ptr) != NULL) {                            \
        (ptr)->code = (err).code;                   \
        (ptr)->origin = (err).origin;               \
        strcpy((ptr)->message, (err).message);      \
    }                                               \
} while (0)

/*****************************************************************************/

/**
 * @brief Error parameter identifier.
 */
#define ARCHI_ERROR_PARAM       archi_parameter_error

/**
 * @brief Error parameter declaration for function parameter lists.
 */
#define ARCHI_ERROR_PARAM_DECL  archi_error_t *const ARCHI_ERROR_PARAM

/**
 * @brief Fill the error parameter with error info.
 */
#define ARCHI_ERROR_SET(err_code, ...)  \
    ARCHI_ERROR_VAR_SET(ARCHI_ERROR_PARAM, (err_code), __VA_ARGS__)

/**
 * @brief Unset error info in the parameter.
 */
#define ARCHI_ERROR_UNSET() \
    ARCHI_ERROR_VAR_UNSET(ARCHI_ERROR_PARAM)

/**
 * @brief Set success status to the error parameter.
 */
#define ARCHI_ERROR_RESET() \
    ARCHI_ERROR_VAR_RESET(ARCHI_ERROR_PARAM)

/**
 * @brief Assign error to the error parameter.
 */
#define ARCHI_ERROR_ASSIGN(err) \
    ARCHI_ERROR_VAR_ASSIGN(ARCHI_ERROR_PARAM, (err))

#endif // _ARCHI_BASE_ERROR_TYP_H_

