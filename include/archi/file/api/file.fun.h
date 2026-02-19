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
 * @brief File operations.
 */

#pragma once
#ifndef _ARCHI_FILE_API_FILE_FUN_H_
#define _ARCHI_FILE_API_FILE_FUN_H_

#include "archi/file/api/handle.typ.h"
#include "archi/file/api/file.typ.h"
#include "archi_base/pointer.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Open a file.
 *
 * @return File descriptor.
 */
archi_file_descriptor_t
archi_file_open(
        const char *pathname, ///< [in] Path to opened file.
        archi_file_open_params_t params, ///< [in] File opening parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Close a file descriptor.
 */
void
archi_file_close(
        archi_file_descriptor_t fd, ///< [in] File descriptor.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Create a duplicate descriptor of a file.
 *
 * @return New file descriptor.
 */
archi_file_descriptor_t
archi_file_duplicate(
        archi_file_descriptor_t fd, ///< [in] File descriptor.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Reposition read/write file offset.
 *
 * @return Offset in bytes from the beginning of the file.
 */
long long
archi_file_seek(
        archi_file_descriptor_t fd, ///< [in] File descriptor.
        long long offset, ///< [in] Offset in bytes relative to location specified by whence.
        int whence, ///< [in] Location specifier as in fseek().
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Read data from file.
 *
 * @return Number of bytes read.
 */
size_t
archi_file_read(
        archi_file_descriptor_t fd, ///< [in] File descriptor.
        archi_pointer_t destination, ///< [in,out] Destination buffer.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Write data to file.
 *
 * @return Number of bytes written.
 */
size_t
archi_file_write(
        archi_file_descriptor_t fd, ///< [in] File descriptor.
        archi_pointer_t source, ///< [in] Source buffer.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Synchronize file state with storage device.
 */
void
archi_file_sync(
        archi_file_descriptor_t fd, ///< [in] File descriptor.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_FILE_API_FILE_FUN_H_

