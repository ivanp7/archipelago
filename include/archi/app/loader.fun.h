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
 * @brief Application plugin loading operations.
 */

#pragma once
#ifndef _ARCHI_APP_LOADER_FUN_H_
#define _ARCHI_APP_LOADER_FUN_H_

#include "archi/app/loader.typ.h"
#include "archi/util/status.typ.h"

struct archi_application;

/**
 * @brief Load a library to an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_add_library(
        struct archi_application *app, ///< [in] Application instance.
        archi_app_loader_library_t lib ///< [in] Library to load.
);

/**
 * @brief Unload a library from an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_remove_library(
        struct archi_application *app, ///< [in] Application instance.
        const void *key ///< [in] Library key.
);

/**
 * @brief Add a context interface to an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_add_interface(
        struct archi_application *app, ///< [in] Application instance.
        archi_app_loader_library_symbol_t sym ///< [in] Library symbol to get.
);

/**
 * @brief Remove a context interface from an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_remove_interface(
        struct archi_application *app, ///< [in] Application instance.
        const void *key ///< [in] Interface key.
);

#endif // _ARCHI_APP_LOADER_FUN_H_

