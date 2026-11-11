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
 * @brief The reference counting manager type.
 */

#pragma once
#ifndef _SRC_ARCHI_BASE_REF_COUNT_MANAGER_TYP_H_
#define _SRC_ARCHI_BASE_REF_COUNT_MANAGER_TYP_H_

#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h> // for atomic_uintmax_t, atomic_bool
#else
#  include <stdint.h> // for uintmax_t
#  include <stdbool.h>
#endif


struct archi_ref_count_manager {
#ifndef __STDC_NO_ATOMICS__
    atomic_uintmax_t num_allocations;
    atomic_uintmax_t num_releases;

    atomic_uintmax_t num_increments;
    atomic_uintmax_t num_decrements;

    atomic_bool double_release;
    atomic_bool negative_count;
#else
    uintmax_t num_allocations;
    uintmax_t num_releases;

    uintmax_t num_increments;
    uintmax_t num_decrements;

    bool double_release;
    bool negative_count;
#endif
};

#endif // _SRC_ARCHI_BASE_REF_COUNT_MANAGER_TYP_H_

