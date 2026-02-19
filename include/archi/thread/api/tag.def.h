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
#ifndef _ARCHI_THREAD_API_TAG_DEF_H_
#define _ARCHI_THREAD_API_TAG_DEF_H_

#define ARCHI_POINTER_DATA_TAG__THREAD_GROUP        0x40 ///< Data type tag for archi_thread_group_t.
#define ARCHI_POINTER_DATA_TAG__THREAD_LFQUEUE      0x41 ///< Data type tag for archi_thread_lfqueue_t.

#define ARCHI_POINTER_FUNC_TAG__THREAD_WORK         0x40 ///< Function type tag for archi_thread_group_work_func_t.
#define ARCHI_POINTER_FUNC_TAG__THREAD_CALLBACK     0x41 ///< Function type tag for archi_thread_group_callback_func_t.

#endif // _ARCHI_THREAD_API_TAG_DEF_H_

