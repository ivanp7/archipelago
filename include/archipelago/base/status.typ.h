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
 * @brief Type for status codes.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_STATUS_TYP_H_
#define _ARCHIPELAGO_BASE_STATUS_TYP_H_

/**
 * @def ARCHI_STATUS_NUM_INFO_BITS
 * @brief Number of bits reserved for the information portion of a status code.
 *
 * Status codes < 0 are split into two fields:
 *   - low‑order ARCHI_STATUS_NUM_INFO_BITS bits: error/info code
 *   - high‑order bits: module ID
 */
#define ARCHI_STATUS_NUM_INFO_BITS  16

/**
 * @enum archi_status
 * @brief Standardized status‐code type for library functions.
 *
 * Status codes encode success, generic errors, and module‐ or
 * function‐specific failures in a single integer:
 *   - code <  0 : error
 *   - code == 0 : success
 *   - code >  0 : function‐specific non‐fatal “failure” or special condition
 *
 * Negative error codes are split into two parts:
 *   - high‐order bits :  module ID
 *   - low‐order bits  :  error info within that module
 *
 * This ensures that each library module can define its own error codes
 * without colliding with other modules or with the generic errors below.
 *
 * @note A return value of zero always indicates success.
 * @note Positive return values are interpreted by each function’s documentation.
 */
typedef enum archi_status {
    // Generic error codes (module ID == 0)
    ARCHI_STATUS_EFAILURE   = -1,   ///< Generic failure (callback function returned positive status code).

    ARCHI_STATUS_EMISUSE    = -2,   ///< Incorrect API usage (invalid argument or input).
    ARCHI_STATUS_EINTERFACE = -3,   ///< Required interface pointer was NULL.
    ARCHI_STATUS_EKEY       = -4,   ///< Invalid data key.
    ARCHI_STATUS_EVALUE     = -5,   ///< Invalid data value.

    ARCHI_STATUS_ENOMEMORY  = -8,   ///< Memory allocation failure.
    ARCHI_STATUS_ERESOURCE  = -16,  ///< System resource operation failure.

    ARCHI_STATUS_ENOTIMPL   = -128, ///< Feature or function not implemented.
} archi_status_t;

/**
 * @def ARCHI_STATUS_EBASE(module_id)
 * @brief Compute the error base code for a specific module.
 *
 * Error codes combine a module ID and information bits.
 * This macro generates the negative “base” value for a given module by
 * shifting its positive ID into the high‐order bits of the code.
 * A module-specific error code is defined as:
 *
 *   error_code = ARCHI_STATUS_EBASE(module_id) | error_info_bits;
 *
 * where 0 < error_info_bits < (1 << ARCHI_STATUS_NUM_INFO_BITS).
 *
 * @param module_id
 *   Positive integer identifying the module (must fit in the high bits).
 * @return
 *   A negative status code base for the specified module, ready to OR
 *   with positive info bits.
 *
 * @note
 *   The info portion of the code must not exceed
 *   ARCHI_STATUS_NUM_INFO_BITS bits.
 */
#define ARCHI_STATUS_EBASE(module_id) \
    (-(archi_status_t)(module_id) << ARCHI_STATUS_NUM_INFO_BITS)

/**
 * @def ARCHI_STATUS_ERROR_MODULE_ID(error)
 * @brief Extract the module identifier from a negative status code.
 *
 * For error codes < 0, the high‐order bits encode the module ID
 * (shifted by ARCHI_STATUS_NUM_INFO_BITS). This macro recovers
 * the original positive module ID:
 *
 *   module_id = - ( error >> ARCHI_STATUS_NUM_INFO_BITS );
 *
 * @param error
 *   A non‐positive status code produced by ARCHI_STATUS_EBASE().
 * @return
 *   The positive module ID that generated this error code.
 */
#define ARCHI_STATUS_ERROR_MODULE_ID(error) \
    (-((error) >> ARCHI_STATUS_NUM_INFO_BITS))

/**
 * @def ARCHI_STATUS_ERROR_INFO_BITS(error)
 * @brief Extract the information portion from a negative status code.
 *
 * Status codes < 0 use the low‐order ARCHI_STATUS_NUM_INFO_BITS bits
 * to encode module‐specific information. This macro masks off the
 * high bits and returns the info field:
 *
 *   info = error & ((1 << ARCHI_STATUS_NUM_INFO_BITS) - 1);
 *
 * @param error
 *   A non‐positive status code created via ARCHI_STATUS_EBASE().
 * @return
 *   The positive info code (0 .. (1<<ARCHI_STATUS_NUM_INFO_BITS)-1).
 */
#define ARCHI_STATUS_ERROR_INFO_BITS(error) \
    ((error) & ((1 << ARCHI_STATUS_NUM_INFO_BITS) - 1))

/**
 * @def ARCHI_STATUS_TO_ERROR(status)
 * @brief Normalize any status code into an error code.
 *
 * Converts a generic {@code status} into a non‑positive error code.
 * If {@code status} is already non‑positive (zero or negative), it is
 * returned unchanged. Otherwise a generic failure code is returned:
 *
 *   error = (status <= 0) ? status : ARCHI_STATUS_EFAILURE;
 *
 * This ensures that all positive “success” or informational codes
 * become a standardized error code when an error is expected.
 *
 * @param status
 *   A generic status or return code (may be positive, zero, or negative).
 * @return
 *   If {@code status} ≤ 0: the original {@code status}.
 *   If {@code status} > 0: the predefined failure code
 *   {@code ARCHI_STATUS_EFAILURE} (a non‑positive value).
 */
#define ARCHI_STATUS_TO_ERROR(status) \
    ((status) <= 0 ? (status) : ARCHI_STATUS_EFAILURE)

#endif // _ARCHIPELAGO_BASE_STATUS_TYP_H_

