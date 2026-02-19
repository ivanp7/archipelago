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
 * @brief Data and function type tags.
 */

#pragma once
#ifndef _ARCHI_BASE_TAG_DEF_H_
#define _ARCHI_BASE_TAG_DEF_H_

#define ARCHI_POINTER_DATA_TAG__POINTER     0x02 ///< Data type tag for archi_pointer_t.
#define ARCHI_POINTER_DATA_TAG__RCPOINTER   0x03 ///< Data type tag for archi_rcpointer_t.
#define ARCHI_POINTER_DATA_TAG__KVLIST      0x06 ///< Data type tag for archi_kvlist_t.
#define ARCHI_POINTER_DATA_TAG__KRCVLIST    0x07 ///< Data type tag for archi_krcvlist_t.

#define ARCHI_POINTER_FUNC_TAG__STRING_COMP 0x04 ///< Function type tag for archi_string_comp_func_t.

#endif // _ARCHI_BASE_TAG_DEF_H_

