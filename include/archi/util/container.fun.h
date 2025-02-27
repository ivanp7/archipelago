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
 * @brief Operations on containers.
 */

#pragma once
#ifndef _ARCHI_UTIL_CONTAINER_FUN_H_
#define _ARCHI_UTIL_CONTAINER_FUN_H_

#include "archi/util/container.typ.h"

/**
 * @brief Insert an element into container.
 *
 * @return Status code.
 */
archi_status_t
archi_container_insert(
        archi_container_t container, ///< [in] Container.

        const char *restrict key, ///< [in] Element key.
        void *restrict element ///< [in] Element value.
);

/**
 * @brief Remove an element form container.
 *
 * @return Status code.
 */
archi_status_t
archi_container_remove(
        archi_container_t container, ///< [in] Container.

        const char *restrict key, ///< [in] Element key.
        void *restrict *restrict element ///< [out] Place for element value.
);

/**
 * @brief Extract an element from container.
 *
 * @return Status code.
 */
archi_status_t
archi_container_extract(
        archi_container_t container, ///< [in] Container.

        const char *restrict key, ///< [in] Element key.
        void *restrict *restrict element ///< [out] Place for element value.
);

/**
 * @brief Traverse elements of container.
 *
 * @return Status code.
 */
archi_status_t
archi_container_traverse(
        archi_container_t container, ///< [in] Container.

        archi_container_element_func_t func, ///< [in] Traversal function.
        void *restrict func_data ///< [in,out] Traversal function data.
);

#endif // _ARCHI_UTIL_CONTAINER_FUN_H_

