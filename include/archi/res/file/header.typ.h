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
 * @brief Types for file and memory operations.
 */

#pragma once
#ifndef _ARCHI_RES_FILE_HEADER_TYP_H_
#define _ARCHI_RES_FILE_HEADER_TYP_H_

/**
 * @brief Header of a memory-mapped file.
 *
 * If header is an object in mapped memory,
 * header.addr must be equal to &header.
 *
 * (header.end - header.addr) is the full size of mapped memory.
 */
typedef struct archi_file_header {
    void *addr; ///< Address of the mapped memory starting location.
    void *end;  ///< Address of the first location beyond mapped memory end.
} archi_file_header_t;

#endif // _ARCHI_RES_FILE_HEADER_TYP_H_

