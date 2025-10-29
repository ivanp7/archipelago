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
 * @brief File stream operations.
 */

#pragma once
#ifndef _ARCHI_FILE_API_STREAM_FUN_H_
#define _ARCHI_FILE_API_STREAM_FUN_H_

#include "archi/file/api/file.typ.h"
#include "archipelago/base/error.typ.h"

/**
 * @brief Open a file stream.
 *
 * @note The file descriptor becomes owned by the newly created stream.
 * It must not be closed using archi_file_close(),
 * otherwise it will invoke undefined behavior.
 *
 * @return File stream.
 */
archi_file_stream_t
archi_file_stream_open(
        archi_file_descriptor_t fd, ///< [in] File descriptor.
        archi_file_open_params_t params, ///< [in] File opening parameters.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Close a file stream and its underlying descriptor.
 */
void
archi_file_stream_close(
        archi_file_stream_t file, ///< [in] File stream.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Obtain file descriptor from a file stream.
 *
 * @return File descriptor belonging to stream.
 */
archi_file_descriptor_t
archi_file_stream_descriptor(
        archi_file_stream_t file ///< [in] File stream.
);

#endif // _ARCHI_FILE_API_STREAM_FUN_H_

