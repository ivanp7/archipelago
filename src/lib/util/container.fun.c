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

#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <stddef.h> // for NULL

archi_status_t
archi_container_insert(
        archi_container_t container,

        const char *restrict key,
        void *restrict element)
{
    if ((container.data == NULL) || (container.interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (container.interface->insert_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    return container.interface->insert_fn(container.data, key, element);
}

archi_status_t
archi_container_remove(
        archi_container_t container,

        const char *restrict key,
        void *restrict *restrict element)
{
    if ((container.data == NULL) || (container.interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (container.interface->remove_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    return container.interface->remove_fn(container.data, key, element);
}

archi_status_t
archi_container_extract(
        archi_container_t container,

        const char *restrict key,
        void *restrict *restrict element)
{
    if ((container.data == NULL) || (container.interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (container.interface->extract_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    return container.interface->extract_fn(container.data, key, element);
}

archi_status_t
archi_container_traverse(
        archi_container_t container,

        archi_container_element_func_t func,
        void *restrict func_data)
{
    if ((container.data == NULL) || (container.interface == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (container.interface->traverse_fn == NULL)
        return ARCHI_ERROR_INTERFACE;

    if (func == NULL)
        return ARCHI_ERROR_MISUSE;

    return container.interface->traverse_fn(container.data, func, func_data);
}

