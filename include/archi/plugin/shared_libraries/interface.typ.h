/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Types for shared libraries plugin.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_SHARED_LIBRARIES_INTERFACE_TYP_H_
#define _ARCHI_PLUGIN_SHARED_LIBRARIES_INTERFACE_TYP_H_

/**
 * @brief Shared library configuration.
 */
typedef struct archi_shared_library_config {
    char *pathname; ///< Pathname of library file.
} archi_shared_library_config_t;

#endif // _ARCHI_PLUGIN_SHARED_LIBRARIES_INTERFACE_TYP_H_

