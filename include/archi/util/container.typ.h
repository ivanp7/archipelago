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
 * @brief Types for containers.
 */

#pragma once
#ifndef _ARCHI_UTIL_CONTAINER_TYP_H_
#define _ARCHI_UTIL_CONTAINER_TYP_H_

#include "archi/util/status.typ.h"

/**
 * @brief Declare/define container element function.
 *
 * @return Status code.
 */
#define ARCHI_CONTAINER_ELEMENT_FUNC(name) archi_status_t name( \
        const void *const restrict key, /* Element key. */ \
        void *const restrict element, /* Element value. */ \
        void *const restrict data) /* Function data. */

/**
 * @brief Container element function.
 */
typedef ARCHI_CONTAINER_ELEMENT_FUNC((*archi_container_element_func_t));

/*****************************************************************************/

/**
 * @brief Declare/define container element insertion function.
 *
 * @return Status code.
 */
#define ARCHI_CONTAINER_INSERT_FUNC(func_name) archi_status_t func_name( \
        void *const restrict container, /* Container. */ \
        const void *const restrict key, /* Element key. */ \
        void *const restrict element) /* Element value. */

/**
 * @brief Container element insertion function.
 */
typedef ARCHI_CONTAINER_INSERT_FUNC((*archi_container_insert_func_t));

/**
 * @brief Declare/define container element removal function.
 *
 * @return Status code.
 */
#define ARCHI_CONTAINER_REMOVE_FUNC(func_name) archi_status_t func_name( \
        void *const restrict container, /* Container. */ \
        const void *const restrict key, /* Element key. */ \
        void *restrict *const restrict element) /* Element value. */

/**
 * @brief Container element removal function.
 */
typedef ARCHI_CONTAINER_REMOVE_FUNC((*archi_container_remove_func_t));

/**
 * @brief Declare/define container element extraction function.
 *
 * @return Status code.
 */
#define ARCHI_CONTAINER_EXTRACT_FUNC(func_name) archi_status_t func_name( \
        void *const restrict container, /* Container. */ \
        const void *const restrict key, /* Element key. */ \
        void *restrict *const restrict element) /* Element value. */

/**
 * @brief Container element extraction function.
 */
typedef ARCHI_CONTAINER_EXTRACT_FUNC((*archi_container_extract_func_t));

/**
 * @brief Declare/define container element traversal function.
 *
 * @return Status code.
 */
#define ARCHI_CONTAINER_TRAVERSE_FUNC(func_name) archi_status_t func_name( \
        void *const restrict container, /* Container. */ \
        archi_container_element_func_t func, /* Traversal function. */ \
        void *const restrict func_data) /* Traversal function data. */

/**
 * @brief Container element traversal function.
 */
typedef ARCHI_CONTAINER_TRAVERSE_FUNC((*archi_container_traverse_func_t));

/*****************************************************************************/

/**
 * @brief Container interface functions.
 */
typedef struct archi_container_interface {
    archi_container_insert_func_t insert_fn;        ///< Element insertion function.
    archi_container_remove_func_t remove_fn;        ///< Element removal function.
    archi_container_extract_func_t extract_fn;      ///< Element extraction function.
    archi_container_traverse_func_t traverse_fn;    ///< Container traversal function.
} archi_container_interface_t;

/**
 * @brief A container.
 */
typedef struct archi_container {
    void *data; ///< Container data.
    const archi_container_interface_t *interface; ///< Container interface.
} archi_container_t;

#endif // _ARCHI_UTIL_CONTAINER_TYP_H_

