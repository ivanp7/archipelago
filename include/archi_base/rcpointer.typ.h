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
 * @brief The reference-counted generic pointer type.
 */

#pragma once
#ifndef _ARCHI_BASE_RCPOINTER_TYP_H_
#define _ARCHI_BASE_RCPOINTER_TYP_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/ref_count.typ.h"


/**
 * @struct archi_rcpointer_t
 * @brief Generic wrapper for reference counted data/function pointers with attributes.
 *
 * This type extends `archi_pointer_t` with a reference counter of the pointee.
 */
typedef struct archi_rcpointer {
    union {
        archi_pointer_t p; ///< Managed pointer.
        struct {
            union {
                archi_data_t ptr;      ///< Pointer to writable data.
                archi_rodata_t cptr;   ///< Pointer to read-only data.
                archi_function_t fptr; ///< Pointer to function.
            };
            archi_pointer_attr_t attr; ///< Pointer attributes.
        };
    };
    archi_reference_count_t ref_count; ///< Reference counter of the pointee.
} archi_rcpointer_t;

#endif // _ARCHI_BASE_RCPOINTER_TYP_H_

