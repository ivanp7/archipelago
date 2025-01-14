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
 * @brief Macros for flexible array members.
 */

#pragma once
#ifndef _ARCHI_UTIL_FLEXIBLE_DEF_H_
#define _ARCHI_UTIL_FLEXIBLE_DEF_H_

/**
 * @brief Size of a structure with a flexible array member.
 */
#define ARCHI_FLEXIBLE_SIZEOF(type, member, num_of) \
    (sizeof(type*) + sizeof(((type*)NULL)->member[0]) * (num_of))

#endif // _ARCHI_UTIL_FLEXIBLE_DEF_H_

