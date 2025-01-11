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
 * @brief Application plugin loading operations.
 */

#pragma once
#ifndef _ARCHI_APP_LOADER_FUN_H_
#define _ARCHI_APP_LOADER_FUN_H_

#include "archi/util/container.typ.h"

/**
 * @brief Container element function for loading of a library.
 *
 * Container element type is expected to be archi_app_loader_library_t.
 */
ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_load_library);

/**
 * @brief Container element function for unloading of a library.
 */
ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_unload_library);

/**
 * @brief Container element function for getting a library content.
 *
 * Container element type is expected to be archi_app_loader_library_symbol_t.
 */
ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_get_library_content);

/**
 * @brief Data for library content getter function.
 */
typedef struct archi_app_get_library_content_data {
    archi_container_t contents; ///< Ponters to symbols of loaded libraries.
    archi_container_t loaded_libraries; ///< Loaded libraries.
} archi_app_get_library_content_data_t;

#endif // _ARCHI_APP_LOADER_FUN_H_

