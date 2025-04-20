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
 * @brief File operations.
 */

#pragma once
#ifndef _ARCHI_RES_FILE_INTERFACE_FUN_H_
#define _ARCHI_RES_FILE_INTERFACE_FUN_H_

#include "archi/res/file/interface.typ.h"

/**
 * @brief Get page size.
 *
 * @return Page size.
 */
size_t
archi_file_page_size(void);

/**
 * @brief Open a file.
 *
 * @return File descriptor.
 */
int
archi_file_open(
        archi_file_open_params_t params ///< [in] File opening parameters.
);

/**
 * @brief Close a file.
 *
 * @return True if there were no errors, otherwise false.
 */
bool
archi_file_close(
        int fd ///< [in] File descriptor.
);

/**
 * @brief Map a file into memory.
 *
 * If config.size is 0, it means "until the end of the file".
 *
 * @return Mapped memory address or NULL in case of failure.
 */
void*
archi_file_map(
        int fd, ///< [in] File descriptor.
        archi_file_map_params_t params, ///< [in] File mapping parameters.
        size_t *size /// [out] Mapped memory size.
);

/**
 * @brief Unmap a memory-mapped file.
 *
 * @return True on success, otherwise false.
 */
bool
archi_file_unmap(
        void *mm,   ///< [in] Mapped memory.
        size_t size ///< [in] Size of mapped memory.
);

#endif // _ARCHI_RES_FILE_INTERFACE_FUN_H_

